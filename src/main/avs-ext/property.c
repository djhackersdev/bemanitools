#define LOG_MODULE "avs-ext-property"

#include <windows.h>

#include <errno.h>
#include <stdio.h>

#include "avs-ext/error.h"
#include "avs-ext/property-internal.h"
#include "avs-ext/property-node.h"
#include "avs-ext/property.h"

#include "iface-core/log.h"

#include "imports/avs.h"

#include "util/mem.h"

#define AVS_PROPERTY_STRUCTURE_META_SIZE 576

typedef void (*avs_ext_property_rewinder_t)(uint32_t context);

struct avs_ext_property_str_read_handle {
    const char *buffer;
    size_t buffer_len;
    size_t offset;
};

static void _avs_ext_property_free(core_property_t **property_);

static size_t
_avs_ext_property_str_read(uint32_t context, void *bytes, size_t nbytes)
{
    int result;
    struct avs_ext_property_str_read_handle *handle;

    result = 0;
    handle = TlsGetValue(context);

    if (handle->offset < handle->buffer_len) {
        result = min(nbytes, handle->buffer_len - handle->offset);
        memcpy(bytes, (const void *) (handle->buffer + handle->offset), result);
        handle->offset += result;
    }

    return result;
}

static void _avs_ext_property_str_rewind(uint32_t context)
{
    struct avs_ext_property_str_read_handle *handle;

    handle = TlsGetValue(context);
    handle->offset = 0;
}

static size_t
_avs_ext_property_fread(uint32_t context, void *bytes, size_t nbytes)
{
    FILE *f;

    f = TlsGetValue(context);

    return fread(bytes, 1, nbytes, f);
}

static void _avs_ext_property_frewind(uint32_t context)
{
    FILE *f = TlsGetValue(context);
    rewind(f);
}

static core_property_result_t _avs_ext_property_do_load(
    avs_reader_t reader,
    avs_ext_property_rewinder_t rewinder,
    uint32_t context,
    const char *name,
    avs_ext_property_internal_property_t **property)
{
    struct property *prop;
    void *buffer;
    int nbytes;

    nbytes = property_read_query_memsize(reader, context, 0, 0);

    if (nbytes < 0) {
        return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
    }

    buffer = xmalloc(nbytes);
    prop = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        nbytes);

    if (!prop) {
        free(buffer);
        return CORE_PROPERTY_RESULT_ERROR_ALLOC;
    }

    rewinder(context);

    if (!property_insert_read(prop, 0, reader, context)) {
        property_destroy(prop);
        free(buffer);
        return CORE_PROPERTY_RESULT_ERROR_READ;
    }

    *property = xmalloc(sizeof(avs_ext_property_internal_property_t));
    (*property)->property = prop;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t
_avs_ext_property_create(size_t size, core_property_t **property_)
{
    avs_ext_property_internal_property_t *property;
    void *buffer;
    struct property *prop;

    if (size > UINT32_MAX) {
        return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
    }

    buffer = xmalloc(size);
    prop = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        (uint32_t) size);

    if (!prop) {
        free(buffer);
        return CORE_PROPERTY_RESULT_ERROR_ALLOC;
    }

    *property_ = xmalloc(sizeof(core_property_t));

    property = (avs_ext_property_internal_property_t *) (*property_);
    property->property = prop;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t
_avs_ext_property_file_load(const char *path, core_property_t **property)
{
    FILE *file;
    uint32_t f_keyhole;
    core_property_result_t result;

    file = fopen(path, "r");

    if (!file) {
        switch (errno) {
            case EACCES:
                return CORE_PROPERTY_RESULT_ERROR_PERMISSIONS;
            case ENOENT:
                return CORE_PROPERTY_RESULT_NOT_FOUND;
            default:
                return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
        }
    }

    /* AVS callbacks are only given a 32-bit context parameter, even in 64-bit
       builds of AVS. We allocate a 32-bit TLS key and pass the context in this
       manner instead. Inefficient, but it works. */

    f_keyhole = TlsAlloc();
    TlsSetValue(f_keyhole, file);

    result = _avs_ext_property_do_load(
        _avs_ext_property_fread,
        _avs_ext_property_frewind,
        f_keyhole,
        path,
        (avs_ext_property_internal_property_t **) property);

    TlsFree(f_keyhole);

    fclose(file);

    return result;
}

static core_property_result_t
_avs_ext_property_str_load(const char *str, core_property_t **property)
{
    struct avs_ext_property_str_read_handle read_handle;
    uint32_t s_keyhole;
    core_property_result_t result;

    read_handle.buffer = str;
    read_handle.buffer_len = strlen(str);
    read_handle.offset = 0;

    s_keyhole = TlsAlloc();
    TlsSetValue(s_keyhole, &read_handle);

    result = _avs_ext_property_do_load(
        _avs_ext_property_str_read,
        _avs_ext_property_str_rewind,
        s_keyhole,
        "<string>",
        (avs_ext_property_internal_property_t **) property);

    TlsFree(s_keyhole);

    return result;
}

static core_property_result_t
_avs_ext_property_size(const core_property_t *property_, size_t *size)
{
    avs_ext_property_internal_property_t *property;
    avs_error error;

    property = (avs_ext_property_internal_property_t *) property_;

    // Returns the size of the actual data in the property structure only
    // Hence, using that size only, allocating another buffer for a copy
    // of this might fail or copying the data will fail because the buffer
    // is too small
    error = property_query_size(property->property);

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
    }

    // Hack: *2 to have enough space and not cut off data when cloning/copying
    // property data because...reasons? I haven't figured this one out and
    // there doesn't seem to be an actual API call for that to return the
    // "true" size that allows the caller to figure out how much memory
    // they have to allocate to create a copy of the property structure
    // with property_create and
    *size = (AVS_PROPERTY_STRUCTURE_META_SIZE + error) * 2;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t _avs_ext_property_clone(
    const core_property_t *property_, core_property_t **property_cloned_)
{
    avs_ext_property_internal_property_t *property;
    avs_ext_property_internal_property_t *property_cloned;

    size_t size;
    core_property_result_t result;

    struct property_node *node;
    avs_error error;

    property = (avs_ext_property_internal_property_t *) property_;

    result = _avs_ext_property_size(property_, &size);

    if (CORE_PROPERTY_RESULT_IS_ERROR(result)) {
        return result;
    }

    result = _avs_ext_property_create(size, property_cloned_);

    property_cloned =
        (avs_ext_property_internal_property_t *) *property_cloned_;

    if (CORE_PROPERTY_RESULT_IS_ERROR(result)) {
        return result;
    }

    node = property_search(property->property, NULL, "/");

    if (!node) {
        error = property_get_error(property_cloned->property);

        if (AVS_IS_ERROR(error)) {
            _avs_ext_property_free(property_cloned_);
            return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
        }
    }

    if (!property_node_clone(property_cloned->property, NULL, node, true)) {
        error = property_get_error(property_cloned->property);

        if (AVS_IS_ERROR(error)) {
            _avs_ext_property_free(property_cloned_);
            return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
        }
    }

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static void _avs_ext_property_log(
    const core_property_t *property_, bt_core_log_message_t log_message)
{
    avs_ext_property_internal_node_t node;
    core_property_node_api_t node_api;

    node.property = (avs_ext_property_internal_property_t *) (property_);
    node.node = property_search(node.property->property, NULL, "/");

    if (node.node == NULL) {
        log_message(LOG_MODULE, "<EMPTY>");
        return;
    }

    avs_ext_property_node_core_api_get(&node_api);

    node_api.v1.log((const core_property_node_t *) &node, log_message);
}

static core_property_result_t _avs_ext_property_root_node_get(
    const core_property_t *property_, core_property_node_t *node_)
{
    avs_ext_property_internal_property_t *property;
    avs_ext_property_internal_node_t *node;

    property = (avs_ext_property_internal_property_t *) property_;
    node = (avs_ext_property_internal_node_t *) node_;

    memset(node, 0, sizeof(avs_ext_property_internal_node_t));

    node->property = property;
    node->node = property_search(property->property, NULL, "/");

    if (node->node == NULL) {
        return CORE_PROPERTY_RESULT_NOT_FOUND;
    } else {
        return CORE_PROPERTY_RESULT_SUCCESS;
    }
}

static core_property_result_t _avs_ext_property_other_node_insert(
    core_property_t *property_, const core_property_node_t *node_)
{
    avs_ext_property_internal_property_t *property;
    avs_ext_property_internal_node_t *node;

    property = (avs_ext_property_internal_property_t *) property_;
    node = (avs_ext_property_internal_node_t *) node_;

    if (!property_node_clone(property->property, NULL, node->node, true)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    } else {
        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    }
}

static void _avs_ext_property_free(core_property_t **property_)
{
    avs_ext_property_internal_property_t *property;
    void *buffer;

    property = (avs_ext_property_internal_property_t *) (*property_);

    buffer = property_desc_to_buffer(property->property);
    property_destroy(property->property);
    free(buffer);

    free(*property_);
    *property_ = NULL;
}

static void _avs_ext_property_core_api_get(core_property_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.create = _avs_ext_property_create;
    api->v1.file_load = _avs_ext_property_file_load;
    api->v1.str_load = _avs_ext_property_str_load;
    api->v1.size = _avs_ext_property_size;
    api->v1.clone = _avs_ext_property_clone;
    api->v1.log = _avs_ext_property_log;
    api->v1.root_node_get = _avs_ext_property_root_node_get;
    api->v1.other_node_insert = _avs_ext_property_other_node_insert;
    api->v1.free = _avs_ext_property_free;
}

void avs_ext_property_core_api_set()
{
    core_property_api_t api;

    _avs_ext_property_core_api_get(&api);
    core_property_api_set(&api);
}

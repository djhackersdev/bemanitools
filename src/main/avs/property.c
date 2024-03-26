#include <errno.h>
#include <stdio.h>

#include "avs/error.h"
#include "avs/property-internal.h"
#include "avs/property-node.h"
#include "avs/property.h"

#include "imports/avs.h"

#include "util/mem.h"

#define AVS_PROPERTY_STRUCTURE_META_SIZE 576

typedef void (*avs_property_rewinder_t)(uint32_t context);

struct avs_property_str_read_handle {
    const char *buffer;
    size_t buffer_len;
    size_t offset;
};

static void _avs_property_free(core_property_t **property_);

static int
_avs_property_str_read(uint32_t context, void *bytes, size_t nbytes)
{
    int result;
    struct avs_property_str_read_handle *handle;

    result = 0;
    handle = TlsGetValue(context);

    if (handle->offset < handle->buffer_len) {
        result = min(nbytes, handle->buffer_len - handle->offset);
        memcpy(bytes, (const void *) (handle->buffer + handle->offset), result);
        handle->offset += result;
    }

    return result;
}

static void _avs_property_str_rewind(uint32_t context)
{
    struct avs_property_str_read_handle *handle;
    
    handle = TlsGetValue(context);
    handle->offset = 0;
}

static int _avs_property_fread(uint32_t context, void *bytes, size_t nbytes)
{
    FILE *f;

    f = TlsGetValue(context);

    return fread(bytes, 1, nbytes, f);
}

static void _avs_property_frewind(uint32_t context)
{
    FILE *f = TlsGetValue(context);
    rewind(f);
}

static core_property_result_t _avs_property_do_load(
    avs_reader_t reader, avs_property_rewinder_t rewinder, 
    uint32_t context, const char *name,
    avs_property_internal_property_t **property)
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

    *property = xmalloc(sizeof(avs_property_internal_property_t));
    (*property)->property = prop;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t _avs_property_create(size_t size, core_property_t **property_)
{
    avs_property_internal_property_t *property;
    void *buffer;
    struct property *prop;
    int nbytes;

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

    *property_ = xmalloc(sizeof(avs_property_internal_property_t));
    
    property = (avs_property_internal_property_t*) (*property_);
    property->property = prop;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t _avs_property_file_load(const char *path, core_property_t **property)
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

    result = _avs_property_do_load(
        _avs_property_fread, _avs_property_frewind, f_keyhole, path, (avs_property_internal_property_t**) property);

    TlsFree(f_keyhole);

    fclose(file);

    return result;
}

static core_property_result_t _avs_property_str_load(const char *str, core_property_t **property)
{
    struct avs_property_str_read_handle read_handle;
    uint32_t s_keyhole;
    core_property_result_t result;

    read_handle.buffer = str;
    read_handle.buffer_len = strlen(str);
    read_handle.offset = 0;

    s_keyhole = TlsAlloc();
    TlsSetValue(s_keyhole, &read_handle);

    result = property_util_do_load(
        _avs_property_str_read,
        _avs_property_str_rewind,
        s_keyhole,
        "<string>",
        (avs_property_internal_property_t**) property);

    TlsFree(s_keyhole);

    return result;
}

static core_property_result_t _avs_property_size(const core_property_t *property_, size_t *size)
{
    avs_property_internal_property_t *property;
    avs_error error;

    property = (avs_property_internal_property_t*) property_;

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

static core_property_result_t _avs_property_clone(const core_property_t *property_, core_property_t **property_cloned_)
{
    avs_property_internal_property_t *property;
    avs_property_internal_property_t *property_cloned;

    size_t size;
    core_property_result_t result;

    struct property_node *node;
    struct property_node *node_clone;
    avs_error error;

    property = (avs_property_internal_property_t*) property_;
    property_cloned = (avs_property_internal_property_t*) property_cloned_;

    result = avs_property_size(property_, &size);

    if (CORE_PROPERTY_RESULT_IS_ERROR(result)) {
        return result;
    }

    result = avs_property_create(size, property_cloned_);

    if (CORE_PROPERTY_RESULT_IS_ERROR(result)) {
        return result;
    }

    node = property_search(property->property, NULL, "/");
    node_clone = property_search(property_cloned->property, NULL, "/");

    if (!property_node_clone(property_cloned->property, node_clone, node, true)) {
        error = property_get_error(property_cloned->property);

        if (AVS_IS_ERROR(error)) {
            _avs_property_free(*property_cloned_);
            return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
        }
    }

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static void _avs_property_log(const core_property_t *property_, core_log_message_t log_impl)
{
    avs_property_internal_node_t node;
    core_property_node_impl_t node_impl;

    node.property = (avs_property_internal_property_t*) (property_);
    node.node = property_search(node.property, NULL, "/");

    avs_property_node_impl_get(&node_impl);

    node_impl.log(&node, log_impl);
}

static core_property_result_t _avs_property_root_node_get(const core_property_t *property_, core_property_node_t **node_)
{
    avs_property_internal_property_t *property;
    avs_property_internal_node_t *node;

    property = (avs_property_internal_property_t*) (property_);
    node = (avs_property_internal_node_t*) (*node_);

    node->property = property;
    node->node = property_search(property->property, NULL, "/");

    if (node->node == NULL) {
        return CORE_PROPERTY_RESULT_NOT_FOUND;
    } else {
        return CORE_PROPERTY_RESULT_SUCCESS;
    }
}

static void _avs_property_free(core_property_t **property_)
{
    avs_property_internal_property_t *property;
    void *buffer;

    property = (avs_property_internal_property_t*) (*property_);

    buffer = property_desc_to_buffer(property->property);
    property_destroy(property->property);
    free(buffer);

    free(*property_);
    property_ = NULL;
}

void avs_property_impl_get(core_property_impl_t *impl)
{
    log_assert(impl);

    impl->create = _avs_property_create;
    impl->file_load = _avs_property_file_load;
    impl->str_load = _avs_property_str_load;
    impl->size = _avs_property_size;
    impl->clone = _avs_property_clone;
    impl->log = _avs_property_log;
    impl->root_node_get = _avs_property_root_node_get;
    impl->free = _avs_property_free;
}
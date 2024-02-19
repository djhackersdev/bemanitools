#define LOG_MODULE "property-util"

#include <windows.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "avs-util/error.h"

#include "core/log.h"

#include "imports/avs.h"

#include "launcher/property-util.h"

#include "util/mem.h"
#include "util/str.h"

#define PROPERTY_STRUCTURE_META_SIZE 576

typedef void (*rewinder)(uint32_t context);

struct cstring_read_handle {
    const char *buffer;
    size_t buffer_len;
    size_t offset;
};

struct property_util_node_merge_ctx {
    const char *path;
    const struct property_util_node_merge_strategies *strategies;
};

static struct property *property_util_do_load(
    avs_reader_t reader, rewinder rewinder, uint32_t context, const char *name)
{
    struct property *prop;
    void *buffer;
    int nbytes;

    nbytes = property_read_query_memsize(reader, context, 0, 0);

    if (nbytes < 0) {
        log_fatal("%s: Error querying configuration file", name);
    }

    buffer = xmalloc(nbytes);
    prop = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        nbytes);

    if (!prop) {
        log_fatal(
            "%s: Creating property failed: %s",
            name,
            avs_util_property_error_get_and_clear(prop));
    }

    rewinder(context);

    if (!property_insert_read(prop, 0, reader, context)) {
        log_fatal(
            "%s: Error reading configuration file: %s",
            name,
            avs_util_property_error_get_and_clear(prop));
    }

    return prop;
}

static int property_util_fread(uint32_t context, void *bytes, size_t nbytes)
{
    FILE *f;

    f = TlsGetValue(context);

    return fread(bytes, 1, nbytes, f);
}

static void property_util_frewind(uint32_t context)
{
    FILE *f = TlsGetValue(context);
    rewind(f);
}

static void property_util_log_node_tree_rec(
    struct property_node *parent_node, const char *parent_path)
{
    char cur_path[PROPERTY_NODE_PATH_LEN_MAX];
    char cur_node_name[PROPERTY_NODE_NAME_SIZE_MAX];

    struct property_node *child_node;
    enum property_type property_type;

    int8_t value_s8;
    int16_t value_s16;
    int32_t value_s32;
    int64_t value_s64;
    uint8_t value_u8;
    uint16_t value_u16;
    uint32_t value_u32;
    uint64_t value_u64;
    char value_str[4096];
    bool value_bool;

    avs_error error;

    // Carry on the full root path down the node tree
    property_node_name(parent_node, cur_node_name, sizeof(cur_node_name));

    str_cpy(cur_path, sizeof(cur_path), parent_path);
    str_cat(cur_path, sizeof(cur_path), "/");
    str_cat(cur_path, sizeof(cur_path), cur_node_name);

    child_node = property_node_traversal(parent_node, TRAVERSE_FIRST_CHILD);

    // parent node is a leaf node, print all data of it
    if (child_node == NULL) {
        property_type = property_node_type(parent_node);

        switch (property_type) {
            case PROPERTY_TYPE_VOID:
                log_misc("%s: <VOID>", cur_path);
                break;

            case PROPERTY_TYPE_S8:
                property_node_read(
                    parent_node, property_type, &value_s8, sizeof(value_s8));
                log_misc("%s: %" PRId8, cur_path, value_s8);
                break;

            case PROPERTY_TYPE_S16:
                property_node_read(
                    parent_node, property_type, &value_s16, sizeof(value_s16));
                log_misc("%s: %" PRId16, cur_path, value_s16);
                break;

            case PROPERTY_TYPE_S32:
                property_node_read(
                    parent_node, property_type, &value_s32, sizeof(value_s32));
                log_misc("%s: %" PRId32, cur_path, value_s32);
                break;

            case PROPERTY_TYPE_S64:
                property_node_read(
                    parent_node, property_type, &value_s64, sizeof(value_s64));
                log_misc("%s: %" PRId64, cur_path, value_s64);
                break;

            case PROPERTY_TYPE_U8:
                property_node_read(
                    parent_node, property_type, &value_u8, sizeof(value_u8));
                log_misc("%s: %" PRIu8, cur_path, value_u8);
                break;

            case PROPERTY_TYPE_U16:
                property_node_read(
                    parent_node, property_type, &value_u16, sizeof(value_u16));
                log_misc("%s: %" PRIu16, cur_path, value_u16);
                break;

            case PROPERTY_TYPE_U32:
                property_node_read(
                    parent_node, property_type, &value_u32, sizeof(value_u32));
                log_misc("%s: %" PRIu32, cur_path, value_u32);
                break;

            case PROPERTY_TYPE_U64:
                property_node_read(
                    parent_node, property_type, &value_u64, sizeof(value_u64));
                log_misc("%s: %" PRIu64, cur_path, value_u64);
                break;

            case PROPERTY_TYPE_STR:
                property_node_read(
                    parent_node, property_type, value_str, sizeof(value_str));
                log_misc("%s: %s", cur_path, value_str);

                break;

            case PROPERTY_TYPE_BOOL:
                property_node_read(
                    parent_node,
                    property_type,
                    &value_bool,
                    sizeof(value_bool));
                log_misc("%s: %d", cur_path, value_bool);

                break;

            case PROPERTY_TYPE_BIN:
                log_misc("%s: <BINARY>", cur_path);
                break;

            case PROPERTY_TYPE_ATTR:
                error = property_node_read(
                    parent_node, property_type, value_str, sizeof(value_str));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_util_error_str(error));
                }

                log_misc("%s@: %s", cur_path, value_str);

                break;

            case PROPERTY_TYPE_VOID_WITH_ATTRIBUTES:
                log_misc("%s: <VOID>", cur_path);

                child_node =
                    property_node_traversal(parent_node, TRAVERSE_FIRST_ATTR);

                while (child_node) {
                    property_util_log_node_tree_rec(child_node, cur_path);

                    child_node = property_node_traversal(
                        child_node, TRAVERSE_NEXT_SIBLING);
                }

                break;

            case PROPERTY_TYPE_STR_WITH_ATTRIBUTES:
                error = property_node_read(
                    parent_node, property_type, value_str, sizeof(value_str));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_util_error_str(error));
                }

                log_misc("%s: %s", cur_path, value_str);

                child_node =
                    property_node_traversal(parent_node, TRAVERSE_FIRST_ATTR);

                while (child_node) {
                    property_util_log_node_tree_rec(child_node, cur_path);

                    child_node = property_node_traversal(
                        child_node, TRAVERSE_NEXT_SIBLING);
                }

                break;

            default:
                log_misc("%s: <UNKNOWN TYPE> (%d)", cur_path, property_type);
                break;
        }
    } else {
        while (child_node) {
            property_util_log_node_tree_rec(child_node, cur_path);

            child_node =
                property_node_traversal(child_node, TRAVERSE_NEXT_SIBLING);
        }
    }
}

static int
property_util_cstring_read(uint32_t context, void *bytes, size_t nbytes)
{
    int result = 0;
    struct cstring_read_handle *h = TlsGetValue(context);

    if (h->offset < h->buffer_len) {
        result = min(nbytes, h->buffer_len - h->offset);
        memcpy(bytes, (const void *) (h->buffer + h->offset), result);
        h->offset += result;
    }
    return result;
}

static void property_util_cstring_rewind(uint32_t context)
{
    struct cstring_read_handle *h = TlsGetValue(context);
    h->offset = 0;
}

static int property_util_avs_read(uint32_t context, void *bytes, size_t nbytes)
{
    avs_desc desc = (avs_desc) context;
    return avs_fs_read(desc, bytes, nbytes);
}

static void property_util_avs_rewind(uint32_t context)
{
    avs_desc desc = (avs_desc) context;
    avs_fs_lseek(desc, 0, AVS_SEEK_SET);
}

static void _property_util_node_merge_recursive(
    struct property *parent_property,
    struct property_node *parent,
    struct property_node *source,
    void *ctx)
{
    uint8_t i;
    bool consumed;
    struct property_node *result;

    const struct property_util_node_merge_ctx *ctx_;
    struct property_util_node_merge_ctx ctx_next;

    char parent_name[PROPERTY_NODE_NAME_SIZE_MAX];
    char parent_path[PROPERTY_NODE_PATH_LEN_MAX];

    log_assert(source);
    log_assert(ctx);

    ctx_ = (const struct property_util_node_merge_ctx *) ctx;

    log_assert(ctx_->path);
    log_assert(ctx_->strategies);
    log_assert(ctx_->strategies->num > 0);

    // Default to copying to an empty node
    if (!parent) {
        result = property_node_clone(parent_property, NULL, source, true);

        if (!result) {
            log_fatal("Copying '%s' into empty parent failed", ctx_->path);
        }

        return;
    }

    property_node_name(parent, parent_name, sizeof(parent_name));

    str_cpy(parent_path, sizeof(parent_path), ctx_->path);
    str_cat(parent_path, sizeof(parent_path), "/");
    str_cat(parent_path, sizeof(parent_path), parent_name);

    ctx_next.path = parent_path;
    ctx_next.strategies = ctx_->strategies;

    consumed = false;

    // Apply given strategies, one MUST consume
    for (i = 0; i < ctx_->strategies->num; i++) {
        log_assert(ctx_->strategies->entry[i].path);

        // path == "" matches everything
        if (str_eq(ctx_->strategies->entry[i].path, "") ||
            str_eq(ctx_->strategies->entry[i].path, parent_path)) {

            consumed = ctx_->strategies->entry[i].merge_strategy_do(
                parent_property,
                parent,
                source,
                &ctx_next,
                _property_util_node_merge_recursive);

            log_misc(
                "Merge strategy for '%s' consumed: %d",
                ctx_->strategies->entry[i].path,
                consumed);

            if (consumed) {
                break;
            }
        }
    }

    log_assert(consumed);
}

void property_util_log(struct property *property)
{
    property_util_log_node_tree_rec(property_search(property, NULL, "/"), "");
}

void property_util_node_log(struct property_node *node)
{
    property_util_log_node_tree_rec(node, "");
}

struct property *property_util_load(const char *filename)
{
    FILE *f;
    uint32_t f_keyhole;
    struct property *prop;

    log_assert(filename);

    /* AVS callbacks are only given a 32-bit context parameter, even in 64-bit
       builds of AVS. We allocate a 32-bit TLS key and pass the context in this
       manner instead. Inefficient, but it works. */

    f = fopen(filename, "r");

    f_keyhole = TlsAlloc();
    TlsSetValue(f_keyhole, f);

    if (f == NULL) {
        log_fatal("%s: Error opening configuration file", filename);
    }

    prop = property_util_do_load(
        property_util_fread, property_util_frewind, f_keyhole, filename);

    TlsFree(f_keyhole);

    fclose(f);

    return prop;
}

struct property *property_util_avs_fs_load(const char *filename)
{
    avs_desc desc;
    struct property *prop;

    log_assert(filename);

    desc = avs_fs_open(filename, AVS_FILE_READ, AVS_FILE_FLAG_SHARE_READ);

    if (!desc) {
        log_fatal("%s: Error opening configuration file", filename);
    }

    prop = property_util_do_load(
        property_util_avs_read, property_util_avs_rewind, desc, filename);

    avs_fs_close(desc);

    return prop;
}

struct property *property_util_cstring_load(const char *cstring)
{
    uint32_t s_keyhole;
    struct property *prop;

    log_assert(cstring);

    // see above
    struct cstring_read_handle read_handle;
    read_handle.buffer = cstring;
    read_handle.buffer_len = strlen(cstring);
    read_handle.offset = 0;

    s_keyhole = TlsAlloc();
    TlsSetValue(s_keyhole, &read_handle);

    prop = property_util_do_load(
        property_util_cstring_read,
        property_util_cstring_rewind,
        s_keyhole,
        "<string>");

    TlsFree(s_keyhole);

    return prop;
}

struct property *property_util_clone(struct property *property)
{
    struct property *clone;
    size_t size;
    void *buffer;

    struct property_node *node_property;
    struct property_node *node_clone;

    log_assert(property);

    size = property_util_property_query_real_size(property);

    buffer = xmalloc(size);

    clone = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        size);

    if (!clone) {
        log_fatal("Creating property failed");
    }

    node_property = property_search(property, NULL, "/");
    node_clone = property_search(clone, NULL, "/");

    if (!property_node_clone(clone, node_clone, node_property, true)) {
        log_fatal(
            "Cloning property data failed: %s",
            avs_util_property_error_get_and_clear(clone));
    }

    return clone;
}

void property_util_free(struct property *prop)
{
    void *buffer;

    buffer = property_desc_to_buffer(prop);
    property_destroy(prop);
    free(buffer);
}

uint32_t property_util_property_query_real_size(struct property *property)
{
    avs_error size;

    log_assert(property);

    // Returns the size of the actual data in the property structure only
    // Hence, using that size only, allocating another buffer for a copy
    // of this might fail or copying the data will fail because the buffer
    // is too small
    size = property_query_size(property);

    if (AVS_IS_ERROR(size)) {
        log_fatal(
            "Querying property size failed: %s", avs_util_error_str(size));
    }

    // Hack: *2 to have enough space and not cut off data when cloning/copying
    // property data because...reasons? I haven't figured this one out and
    // there doesn't seem to be an actual API call for that to return the
    // "true" size that allows the caller to figure out how much memory
    // they have to allocate to create a copy of the property structure
    // with property_create and
    return (PROPERTY_STRUCTURE_META_SIZE + size) * 2;
}

void property_util_node_u8_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    uint8_t val)
{
    struct property_node *tmp;

    log_assert(node);
    log_assert(name);

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(property, node, PROPERTY_TYPE_U8, name, val);

    if (!tmp) {
        log_fatal(
            "Creating node '%s' failed: %s",
            name,
            property ? avs_util_property_error_get_and_clear(property) :
                       "unknown");
    }
}

void property_util_node_u16_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    uint16_t val)
{
    struct property_node *tmp;

    log_assert(node);
    log_assert(name);

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(property, node, PROPERTY_TYPE_U16, name, val);

    if (!tmp) {
        log_fatal(
            "Creating node '%s' failed: %s",
            name,
            property ? avs_util_property_error_get_and_clear(property) :
                       "unknown");
    }
}

void property_util_node_u32_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    uint32_t val)
{
    struct property_node *tmp;

    log_assert(node);
    log_assert(name);

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(property, node, PROPERTY_TYPE_U32, name, val);

    if (!tmp) {
        log_fatal(
            "Creating node '%s' failed: %s",
            name,
            property ? avs_util_property_error_get_and_clear(property) :
                       "unknown");
    }
}

void property_util_node_str_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    const char *val)
{
    struct property_node *tmp;

    log_assert(node);
    log_assert(name);

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(property, node, PROPERTY_TYPE_STR, name, val);

    if (!tmp) {
        log_fatal(
            "Creating node '%s' failed: %s",
            name,
            property ? avs_util_property_error_get_and_clear(property) :
                       "unknown");
    }
}

void property_util_node_bool_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    bool val)
{
    struct property_node *tmp;

    log_assert(node);
    log_assert(name);

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(property, node, PROPERTY_TYPE_BOOL, name, val);

    if (!tmp) {
        log_fatal(
            "Creating node '%s' failed: %s",
            name,
            property ? avs_util_property_error_get_and_clear(property) :
                       "unknown");
    }
}

void property_util_node_attribute_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    const char *val)
{
    struct property_node *tmp;

    log_assert(node);
    log_assert(name);

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(property, node, PROPERTY_TYPE_ATTR, name, val);
}

struct property *
property_util_many_merge(struct property **properties, size_t count)
{
    struct property *merged_property;
    struct property *tmp;
    int i;

    log_assert(properties);
    log_assert(count > 0);

    merged_property = property_util_clone(properties[0]);

    if (count == 1) {
        return merged_property;
    }

    for (i = 1; i < count; i++) {
        tmp = property_util_merge(merged_property, properties[i]);

        property_util_free(merged_property);
        merged_property = tmp;
    }

    return merged_property;
}

struct property *property_util_node_extract(struct property_node *node)
{
    struct property *property;
    struct property_node *root_node;
    uint32_t size;
    void *buffer;
    struct property_node *result;

    if (!node) {
        return NULL;
    }

    // Hack: Is it even possible to get the size of a (sub-) node without
    // the property? 256kb should be fine for now, even for larger
    // configurations. Obviously, this scales horribly and wastes a lot of
    // memory for most smaller sub-nodes
    size = 1024 * 256;

    buffer = xmalloc(size);
    property = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        size);
    root_node = property_search(property, NULL, "");

    result = property_node_clone(property, root_node, node, true);

    if (!result) {
        log_fatal("Cloning node into empty property failed");
    }

    return property;
}

struct property *
property_util_merge(struct property *parent, struct property *source)
{
    struct property_util_node_merge_strategies strategies;

    log_assert(parent);
    log_assert(source);

    strategies.num = 1;

    strategies.entry[0].path = "";
    strategies.entry[0].merge_strategy_do =
        property_util_node_merge_default_strategy_do;

    return property_util_merge_with_strategies(parent, source, &strategies);
}

struct property *property_util_merge_with_strategies(
    struct property *parent,
    struct property *source,
    const struct property_util_node_merge_strategies *strategies)
{
    struct property_util_node_merge_ctx ctx;
    size_t total_size;
    void *buffer;
    struct property *merged;
    struct property_node *parent_node;
    struct property_node *source_node;

    log_assert(parent);
    log_assert(source);
    log_assert(strategies);

    // We can't estimate how these two are being merged as in how much new
    // data is being inserted from source into parent. Therefore, worse-case
    // estimate memory requirement for no overlap
    total_size = 0;
    total_size += property_util_property_query_real_size(parent);
    total_size += property_util_property_query_real_size(source);

    buffer = xmalloc(total_size);

    merged = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        total_size);

    ctx.path = "";
    ctx.strategies = strategies;

    parent_node = property_search(parent, NULL, "/");

    if (!property_node_clone(merged, NULL, parent_node, true)) {
        log_fatal(
            "Copying parent base failed: %s",
            avs_util_property_error_get_and_clear(merged));
    }

    // Grab parent_node from merged property which is the target one to merge
    // into
    parent_node = property_search(merged, NULL, "/");
    source_node = property_search(source, NULL, "/");

    _property_util_node_merge_recursive(merged, parent_node, source_node, &ctx);

    return merged;
}

bool property_util_node_merge_default_strategy_do(
    struct property *parent_property,
    struct property_node *parent,
    struct property_node *source,
    void *ctx,
    property_util_node_merge_recursion_do_t node_merge_recursion_do)
{
    struct property_node *result;

    struct property_node *parent_child;
    struct property_node *source_child;
    struct property_node *source_child_child;

    char child_node_name[PROPERTY_NODE_NAME_SIZE_MAX];

    log_assert(parent);
    log_assert(source);

    source_child = property_node_traversal(source, TRAVERSE_FIRST_CHILD);

    while (source_child) {
        property_node_name(
            source_child, child_node_name, sizeof(child_node_name));

        parent_child = property_search(NULL, parent, child_node_name);

        if (parent_child) {
            source_child_child =
                property_node_traversal(source_child, TRAVERSE_FIRST_CHILD);

            if (source_child_child) {
                // Continue recursion if there are actually more children
                node_merge_recursion_do(
                    parent_property, parent_child, source_child, ctx);
            } else {
                // Found identical leaf node, remove the matching parent's child
                // and copy the source child over to the parent and terminate
                // the recursion
                property_node_remove(parent_child);
                result = property_node_clone(
                    parent_property, parent, source_child, true);

                if (!result) {
                    log_fatal(
                        "Replacing leaf node '%s' failed", child_node_name);
                }
            }
        } else {
            // Could not find an identical child on parent, copy source
            // recursively to parent
            result = property_node_clone(
                parent_property, parent, source_child, true);

            if (!result) {
                log_fatal("Deep copying child '%s' failed", child_node_name);
            }
        }

        source_child =
            property_node_traversal(source_child, TRAVERSE_NEXT_SIBLING);
    }

    // Default strategy always consumes
    return true;
}

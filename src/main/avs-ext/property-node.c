#define LOG_MODULE "avs-ext-property-node"

#include <inttypes.h>

#include "avs-ext/error.h"
#include "avs-ext/property-internal.h"
#include "avs-ext/property-node.h"

#include "iface-core/log.h"

#include "imports/avs.h"

#include "util/mem.h"
#include "util/str.h"

#define AVS_PROPERTY_STRUCTURE_META_SIZE 576

// Guestimate, should be long enough, I hope?
#define AVS_PROPERTY_NODE_PATH_LEN_MAX 4096
// 256 found in AVS code as size used on property_node_name
#define AVS_PROPERTY_NODE_NAME_SIZE_MAX 256
// Guestimate, should be enough, I hope?
#define AVS_PROPERTY_NODE_ATTR_NAME_SIZE_MAX 128

static void _avs_ext_property_node_log_rec(
    struct property_node *parent_node,
    const char *parent_path,
    bt_core_log_message_t log_message)
{
    char cur_path[AVS_PROPERTY_NODE_PATH_LEN_MAX];
    char cur_node_name[AVS_PROPERTY_NODE_NAME_SIZE_MAX];

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
                log_message(LOG_MODULE, "%s: <VOID>", cur_path);
                break;

            case PROPERTY_TYPE_S8:
                error = property_node_read(
                    parent_node, property_type, &value_s8, sizeof(value_s8));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(LOG_MODULE, "%s: %" PRId8, cur_path, value_s8);
                }

                break;

            case PROPERTY_TYPE_S16:
                error = property_node_read(
                    parent_node, property_type, &value_s16, sizeof(value_s16));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(
                        LOG_MODULE, "%s: %" PRId16, cur_path, value_s16);
                }

                break;

            case PROPERTY_TYPE_S32:
                error = property_node_read(
                    parent_node, property_type, &value_s32, sizeof(value_s32));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(
                        LOG_MODULE, "%s: %" PRId32, cur_path, value_s32);
                }

                break;

            case PROPERTY_TYPE_S64:
                error = property_node_read(
                    parent_node, property_type, &value_s64, sizeof(value_s64));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(
                        LOG_MODULE, "%s: %" PRId64, cur_path, value_s64);
                }

                break;

            case PROPERTY_TYPE_U8:
                error = property_node_read(
                    parent_node, property_type, &value_u8, sizeof(value_u8));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(LOG_MODULE, "%s: %" PRIu8, cur_path, value_u8);
                }

                break;

            case PROPERTY_TYPE_U16:
                error = property_node_read(
                    parent_node, property_type, &value_u16, sizeof(value_u16));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(
                        LOG_MODULE, "%s: %" PRIu16, cur_path, value_u16);
                }

                break;

            case PROPERTY_TYPE_U32:
                error = property_node_read(
                    parent_node, property_type, &value_u32, sizeof(value_u32));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(
                        LOG_MODULE, "%s: %" PRIu32, cur_path, value_u32);
                }

                break;

            case PROPERTY_TYPE_U64:
                error = property_node_read(
                    parent_node, property_type, &value_u64, sizeof(value_u64));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(
                        LOG_MODULE, "%s: %" PRIu64, cur_path, value_u64);
                }

                break;

            case PROPERTY_TYPE_STR:
                error = property_node_read(
                    parent_node, property_type, value_str, sizeof(value_str));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(LOG_MODULE, "%s: %s", cur_path, value_str);
                }

                break;

            case PROPERTY_TYPE_BOOL:
                error = property_node_read(
                    parent_node,
                    property_type,
                    &value_bool,
                    sizeof(value_bool));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(LOG_MODULE, "%s: %d", cur_path, value_bool);
                }

                break;

            case PROPERTY_TYPE_BIN:
                log_message(LOG_MODULE, "%s: <BINARY>", cur_path);
                break;

            case PROPERTY_TYPE_ATTR:
                error = property_node_read(
                    parent_node, property_type, value_str, sizeof(value_str));

                if (AVS_IS_ERROR(error)) {
                    log_fatal(
                        "%s, property read failed: %s",
                        cur_path,
                        avs_ext_error_str(error));
                } else {
                    log_message(LOG_MODULE, "%s@: %s", cur_path, value_str);
                }

                break;

            case PROPERTY_TYPE_VOID_WITH_ATTRIBUTES:
                log_message(LOG_MODULE, "%s: <VOID>", cur_path);

                child_node =
                    property_node_traversal(parent_node, TRAVERSE_FIRST_ATTR);

                while (child_node) {
                    _avs_ext_property_node_log_rec(
                        child_node, cur_path, log_message);

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
                        avs_ext_error_str(error));
                }

                log_message(LOG_MODULE, "%s: %s", cur_path, value_str);

                child_node =
                    property_node_traversal(parent_node, TRAVERSE_FIRST_ATTR);

                while (child_node) {
                    _avs_ext_property_node_log_rec(
                        child_node, cur_path, log_message);

                    child_node = property_node_traversal(
                        child_node, TRAVERSE_NEXT_SIBLING);
                }

                break;

            default:
                log_fatal("%s: <UNKNOWN TYPE> (%d)", cur_path, property_type);
                break;
        }
    } else {
        while (child_node) {
            _avs_ext_property_node_log_rec(child_node, cur_path, log_message);

            child_node =
                property_node_traversal(child_node, TRAVERSE_NEXT_SIBLING);
        }
    }
}

static void _avs_ext_property_node_log(
    const core_property_node_t *node_, bt_core_log_message_t log_messsage)
{
    avs_ext_property_internal_node_t *node;

    node = (avs_ext_property_internal_node_t *) node_;

    _avs_ext_property_node_log_rec(node->node, "", log_messsage);
}

static core_property_node_result_t _avs_ext_property_node_name_get(
    const core_property_node_t *node_, char *name, size_t len)
{
    avs_ext_property_internal_node_t *node;

    node = (avs_ext_property_internal_node_t *) node_;

    property_node_name(node->node, name, len);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t
_avs_ext_property_node_size(const core_property_node_t *node_, size_t *size)
{
    avs_ext_property_internal_node_t *node;
    avs_error error;

    node = (avs_ext_property_internal_node_t *) node_;

    // No idea how to get the size of the memory that this node with all
    // subnodes requires. Use the total size of the property it is owned by.
    // This potentially wastes a lot of memory, but is considered ok for now
    // because the known use-cases are fairly limited regarding usage of this

    // Copy-paste from avs_ext_property_size implementation

    // Returns the size of the actual data in the property structure only
    // Hence, using that size only, allocating another buffer for a copy
    // of this might fail or copying the data will fail because the buffer
    // is too small
    error = property_query_size(node->property->property);

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

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_search(
    const core_property_node_t *node_,
    const char *path,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    node = (avs_ext_property_internal_node_t *) node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

    tmp = property_search(NULL, node->node, path);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    node_out->property = node->property;
    node_out->node = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_next_result_search(
    const core_property_node_t *node_, core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    node = (avs_ext_property_internal_node_t *) node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

    tmp = property_node_traversal(node->node, TRAVERSE_NEXT_SEARCH_RESULT);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    node_out->property = node->property;
    node_out->node = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_child_get(
    const core_property_node_t *node_, core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    node = (avs_ext_property_internal_node_t *) node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

    tmp = property_node_traversal(node->node, TRAVERSE_FIRST_CHILD);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    node_out->property = node->property;
    node_out->node = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_next_sibling_get(
    const core_property_node_t *node_, core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    node = (avs_ext_property_internal_node_t *) node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

    tmp = property_node_traversal(node->node, TRAVERSE_NEXT_SIBLING);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    node_out->property = node->property;
    node_out->node = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_void_create(
    const core_property_node_t *parent_node_,
    const char *key,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_VOID,
        key);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s8_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int8_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_S8,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u8_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint8_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_U8,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s16_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int16_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_S16,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u16_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint16_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_U16,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s32_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int32_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_S32,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u32_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_U32,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s64_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int64_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_S64,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u64_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint64_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_U64,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_bin_create(
    const core_property_node_t *parent_node_,
    const char *key,
    void *data,
    size_t len,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_BIN,
        key,
        data,
        len);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_str_create(
    const core_property_node_t *parent_node_,
    const char *key,
    const char *value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_STR,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_ipv4_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_IP4,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_float_create(
    const core_property_node_t *parent_node_,
    const char *key,
    float value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_FLOAT,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_double_create(
    const core_property_node_t *parent_node_,
    const char *key,
    double value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_DOUBLE,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_attr_create(
    const core_property_node_t *parent_node_,
    const char *key,
    const char *value)
{
    avs_ext_property_internal_node_t *parent_node;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_ATTR,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_bool_create(
    const core_property_node_t *parent_node_,
    const char *key,
    bool value,
    core_property_node_t *node_out_)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_ext_property_internal_node_t *node_out;
    struct property_node *tmp;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;
    node_out = (avs_ext_property_internal_node_t *) node_out_;

    tmp = property_node_create(
        parent_node->property->property,
        parent_node->node,
        PROPERTY_TYPE_BOOL,
        key,
        value);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(avs_ext_property_internal_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s8_read(
    const core_property_node_t *parent_node_, int8_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_S8, value, sizeof(int8_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u8_read(
    const core_property_node_t *parent_node_, uint8_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_U8, value, sizeof(uint8_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s16_read(
    const core_property_node_t *parent_node_, int16_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_S16, value, sizeof(int16_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u16_read(
    const core_property_node_t *parent_node_, uint16_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_U16, value, sizeof(uint16_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s32_read(
    const core_property_node_t *parent_node_, int32_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_S32, value, sizeof(int32_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u32_read(
    const core_property_node_t *parent_node_, uint32_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_U32, value, sizeof(uint32_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_s64_read(
    const core_property_node_t *parent_node_, int64_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_S64, value, sizeof(int64_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_u64_read(
    const core_property_node_t *parent_node_, uint64_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_U64, value, sizeof(uint64_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_bin_read(
    const core_property_node_t *parent_node_, void *value, size_t len)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error =
        property_node_read(parent_node->node, PROPERTY_TYPE_BIN, value, len);

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_str_read(
    const core_property_node_t *parent_node_, char *value, size_t len)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error =
        property_node_read(parent_node->node, PROPERTY_TYPE_STR, value, len);

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_ipv4_read(
    const core_property_node_t *parent_node_, uint32_t *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_IP4, value, sizeof(uint32_t));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_float_read(
    const core_property_node_t *parent_node_, float *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_FLOAT, value, sizeof(float));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_double_read(
    const core_property_node_t *parent_node_, double *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_DOUBLE, value, sizeof(double));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_attr_read(
    const core_property_node_t *parent_node_, const char *key, char *value, size_t len)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;
    size_t attr_key_len;
    char *attr_key;
    struct property_node *node_attr;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    // Append @ denoting an attribute, plus null terminator
    attr_key_len = strlen(key) + 1 + 1;

    attr_key = xmalloc(sizeof(char) * attr_key_len);

    str_cpy(attr_key, attr_key_len, key);
    str_cat(attr_key, attr_key_len, "@");

    node_attr = property_search(NULL, parent_node->node, attr_key);

    if (node_attr == NULL) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    error =
        property_node_read(node_attr, PROPERTY_TYPE_ATTR, value, len);

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_bool_read(
    const core_property_node_t *parent_node_, bool *value)
{
    avs_ext_property_internal_node_t *parent_node;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    error = property_node_read(
        parent_node->node, PROPERTY_TYPE_BOOL, value, sizeof(bool));

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_attr_remove(
    const core_property_node_t *parent_node_, const char *key)
{
    avs_ext_property_internal_node_t *parent_node;
    struct property_node *attr_node;
    size_t attr_key_len;
    char *attr_key;
    avs_error error;

    parent_node = (avs_ext_property_internal_node_t *) parent_node_;

    // Append @ denoting an attribute, plus null terminator
    attr_key_len = strlen(key) + 1 + 1;

    attr_key = xmalloc(sizeof(char) * attr_key_len);

    str_cpy(attr_key, attr_key_len, key);
    str_cat(attr_key, attr_key_len, "@");

    // AVS property treats attributes as a type of node
    attr_node = property_search(NULL, parent_node->node, attr_key);

    if (attr_node == NULL) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    error = property_node_remove(attr_node);

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t
_avs_ext_property_node_remove(const core_property_node_t *node_)
{
    avs_ext_property_internal_node_t *node;
    avs_error error;

    node = (avs_ext_property_internal_node_t *) node_;

    error = property_node_remove(node->node);

    if (AVS_IS_ERROR(error)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _avs_ext_property_node_copy(
    core_property_node_t *dst_node_, const core_property_node_t *src_node_)
{
    avs_ext_property_internal_node_t *dst_node;
    avs_ext_property_internal_node_t *src_node;

    dst_node = (avs_ext_property_internal_node_t *) dst_node_;
    src_node = (avs_ext_property_internal_node_t *) src_node_;

    if (!property_node_clone(
            dst_node->property->property,
            dst_node->node,
            src_node->node,
            true)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    } else {
        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    }
}

void avs_ext_property_node_core_api_get(core_property_node_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.log = _avs_ext_property_node_log;
    api->v1.name_get = _avs_ext_property_node_name_get;
    api->v1.size = _avs_ext_property_node_size;
    api->v1.search = _avs_ext_property_node_search;
    api->v1.next_result_search = _avs_ext_property_node_next_result_search;
    api->v1.child_get = _avs_ext_property_node_child_get;
    api->v1.next_sibling_get = _avs_ext_property_node_next_sibling_get;
    api->v1.void_create = _avs_ext_property_node_void_create;
    api->v1.s8_create = _avs_ext_property_node_s8_create;
    api->v1.u8_create = _avs_ext_property_node_u8_create;
    api->v1.s16_create = _avs_ext_property_node_s16_create;
    api->v1.u16_create = _avs_ext_property_node_u16_create;
    api->v1.s32_create = _avs_ext_property_node_s32_create;
    api->v1.u32_create = _avs_ext_property_node_u32_create;
    api->v1.s64_create = _avs_ext_property_node_s64_create;
    api->v1.u64_create = _avs_ext_property_node_u64_create;
    api->v1.bin_create = _avs_ext_property_node_bin_create;
    api->v1.str_create = _avs_ext_property_node_str_create;
    api->v1.ipv4_create = _avs_ext_property_node_ipv4_create;
    api->v1.float_create = _avs_ext_property_node_float_create;
    api->v1.double_create = _avs_ext_property_node_double_create;
    api->v1.attr_create = _avs_ext_property_node_attr_create;
    api->v1.bool_create = _avs_ext_property_node_bool_create;
    api->v1.s8_read = _avs_ext_property_node_s8_read;
    api->v1.u8_read = _avs_ext_property_node_u8_read;
    api->v1.s16_read = _avs_ext_property_node_s16_read;
    api->v1.u16_read = _avs_ext_property_node_u16_read;
    api->v1.s32_read = _avs_ext_property_node_s32_read;
    api->v1.u32_read = _avs_ext_property_node_u32_read;
    api->v1.s64_read = _avs_ext_property_node_s64_read;
    api->v1.u64_read = _avs_ext_property_node_u64_read;
    api->v1.bin_read = _avs_ext_property_node_bin_read;
    api->v1.str_read = _avs_ext_property_node_str_read;
    api->v1.ipv4_read = _avs_ext_property_node_ipv4_read;
    api->v1.float_read = _avs_ext_property_node_float_read;
    api->v1.double_read = _avs_ext_property_node_double_read;
    api->v1.attr_read = _avs_ext_property_node_attr_read;
    api->v1.bool_read = _avs_ext_property_node_bool_read;
    api->v1.remove = _avs_ext_property_node_remove;
    api->v1.attr_remove = _avs_ext_property_node_attr_remove;
    api->v1.copy = _avs_ext_property_node_copy;
}

void avs_ext_property_node_core_api_set()
{
    core_property_node_api_t api;

    avs_ext_property_node_core_api_get(&api);
    core_property_node_api_set(&api);
}

struct property_node *
avs_ext_property_node_avs_property_node_get(const core_property_node_t *node)
{
    avs_ext_property_internal_node_t *internal_node;

    log_assert(node);

    internal_node = (avs_ext_property_internal_node_t *) node;

    return internal_node->node;
}
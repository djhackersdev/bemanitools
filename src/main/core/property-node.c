#define LOG_MODULE "core-property-node"

#include <string.h>

#include "iface-core/log.h"

#include "main/core/property-node-trace.h"
#include "main/core/property-node.h"

#define CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(func, name)          \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bool _core_property_node_trace_log_enable;
static core_property_node_api_t _core_property_node_api;

static bool _core_property_node_api_is_valid()
{
    return _core_property_node_api.version > 0;
}

void core_property_node_trace_log_enable(bool enable)
{
    _core_property_node_trace_log_enable = enable;
}

void core_property_node_api_set(const core_property_node_api_t *api)
{
    core_property_node_api_t trace_api;

    log_assert(api);

    if (_core_property_node_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.name_get, name_get);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.size, size);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.search, search);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(
            api->v1.next_result_search, next_result_search);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.child_get, child_get);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(
            api->v1.next_sibling_get, next_sibling_get);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.void_create, void_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s8_create, s8_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u8_create, u8_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s16_create, s16_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u16_create, u16_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s32_create, s32_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u32_create, u32_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s64_create, s64_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u64_create, u64_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.bin_create, bin_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.str_create, str_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.ipv4_create, ipv4_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(
            api->v1.float_create, float_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(
            api->v1.double_create, double_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.attr_create, attr_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.bool_create, bool_create);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s8_read, s8_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u8_read, u8_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s16_read, s16_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u16_read, u16_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s32_read, s32_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u32_read, u32_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.s64_read, s64_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.u64_read, u64_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.bin_read, bin_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.str_read, str_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.ipv4_read, ipv4_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.float_read, float_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.double_read, double_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.attr_read, attr_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.bool_read, bool_read);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.remove, remove);
        CORE_PROPERTY_NODE_ASSERT_IMPLEMENTED(api->v1.copy, copy);
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }

    if (_core_property_node_trace_log_enable) {
        log_info("API trace log enabled");

        core_property_node_trace_target_api_set(api);
        core_property_node_trace_core_api_get(&trace_api);

        memcpy(
            &_core_property_node_api,
            &trace_api,
            sizeof(core_property_node_api_t));
    } else {
        memcpy(&_core_property_node_api, api, sizeof(core_property_node_api_t));
    }
}

void core_property_node_api_get(core_property_node_api_t *api)
{
    log_assert(api);
    log_assert(_core_property_node_api_is_valid());

    memcpy(api, &_core_property_node_api, sizeof(core_property_node_api_t));
}

const char *core_property_node_result_to_str(core_property_node_result_t result)
{
    switch (result) {
        case CORE_PROPERTY_NODE_RESULT_SUCCESS:
            return "Success";
        case CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL:
            return "Internal";
        case CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND:
            return "Node not found";
        default:
            return "Undefined error";
    }
}

core_property_node_result_t core_property_node_name_get(
    const core_property_node_t *node, char *name, size_t len)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(node);
    log_assert(name);
    log_assert(len > 0);

    return _core_property_node_api.v1.name_get(node, name, len);
}

core_property_node_result_t
core_property_node_size(const core_property_node_t *node, size_t *size)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(node);
    log_assert(size);

    return _core_property_node_api.v1.size(node, size);
}

core_property_node_result_t core_property_node_search(
    const core_property_node_t *node,
    const char *path,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(node);
    log_assert(path);
    log_assert(node_out);

    return _core_property_node_api.v1.search(node, path, node_out);
}

core_property_node_result_t core_property_node_next_result_search(
    const core_property_node_t *node, core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(node);
    log_assert(node_out);

    return _core_property_node_api.v1.next_result_search(node, node_out);
}

core_property_node_result_t core_property_node_child_get(
    const core_property_node_t *node, core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(node);
    log_assert(node_out);

    return _core_property_node_api.v1.child_get(node, node_out);
}

core_property_node_result_t core_property_node_next_sibling_get(
    const core_property_node_t *node, core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(node);
    log_assert(node_out);

    return _core_property_node_api.v1.next_sibling_get(node, node_out);
}

core_property_node_result_t core_property_node_void_create(
    const core_property_node_t *parent_node,
    const char *key,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.void_create(parent_node, key, node_out);
}

core_property_node_result_t core_property_node_s8_create(
    const core_property_node_t *parent_node,
    const char *key,
    int8_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.s8_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_u8_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint8_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.u8_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_s16_create(
    const core_property_node_t *parent_node,
    const char *key,
    int16_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.s16_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_u16_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint16_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.u16_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_s32_create(
    const core_property_node_t *parent_node,
    const char *key,
    int32_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.s32_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_u32_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.u32_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_s64_create(
    const core_property_node_t *parent_node,
    const char *key,
    int64_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.s64_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_u64_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint64_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.u64_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_bin_create(
    const core_property_node_t *parent_node,
    const char *key,
    void *data,
    size_t len,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);
    log_assert(data);

    return _core_property_node_api.v1.bin_create(
        parent_node, key, data, len, node_out);
}

core_property_node_result_t core_property_node_str_create(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);
    log_assert(value);

    return _core_property_node_api.v1.str_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_ipv4_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.ipv4_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_float_create(
    const core_property_node_t *parent_node,
    const char *key,
    float value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.float_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_double_create(
    const core_property_node_t *parent_node,
    const char *key,
    double value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.double_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_attr_create(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);
    log_assert(value);

    return _core_property_node_api.v1.attr_create(
        parent_node, key, value);
}

core_property_node_result_t core_property_node_bool_create(
    const core_property_node_t *parent_node,
    const char *key,
    bool value,
    core_property_node_t *node_out)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.bool_create(
        parent_node, key, value, node_out);
}

core_property_node_result_t core_property_node_s8_read(
    const core_property_node_t *parent_node, int8_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.s8_read(parent_node, value);
}

core_property_node_result_t core_property_node_u8_read(
    const core_property_node_t *parent_node, uint8_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.u8_read(parent_node, value);
}

core_property_node_result_t core_property_node_s16_read(
    const core_property_node_t *parent_node, int16_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.s16_read(parent_node, value);
}

core_property_node_result_t core_property_node_u16_read(
    const core_property_node_t *parent_node, uint16_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.u16_read(parent_node, value);
}

core_property_node_result_t core_property_node_s32_read(
    const core_property_node_t *parent_node, int32_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.s32_read(parent_node, value);
}

core_property_node_result_t core_property_node_u32_read(
    const core_property_node_t *parent_node, uint32_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.u32_read(parent_node, value);
}

core_property_node_result_t core_property_node_s64_read(
    const core_property_node_t *parent_node, int64_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.s64_read(parent_node, value);
}

core_property_node_result_t core_property_node_u64_read(
    const core_property_node_t *parent_node, uint64_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.u64_read(parent_node, value);
}

core_property_node_result_t core_property_node_bin_read(
    const core_property_node_t *parent_node, void *value, size_t len)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.bin_read(parent_node, value, len);
}

core_property_node_result_t core_property_node_str_read(
    const core_property_node_t *parent_node, char *value, size_t len)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.str_read(parent_node, value, len);
}

core_property_node_result_t core_property_node_ipv4_read(
    const core_property_node_t *parent_node, uint32_t *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.ipv4_read(parent_node, value);
}

core_property_node_result_t core_property_node_float_read(
    const core_property_node_t *parent_node, float *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.float_read(parent_node, value);
}

core_property_node_result_t core_property_node_double_read(
    const core_property_node_t *parent_node, double *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.double_read(parent_node, value);
}

core_property_node_result_t core_property_node_attr_read(
    const core_property_node_t *parent_node, const char *key, char *value, size_t len)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);
    log_assert(value);

    return _core_property_node_api.v1.attr_read(parent_node, key, value, len);
}

core_property_node_result_t core_property_node_bool_read(
    const core_property_node_t *parent_node, bool *value)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(value);

    return _core_property_node_api.v1.bool_read(parent_node, value);
}

core_property_node_result_t
core_property_node_remove(const core_property_node_t *node)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(node);

    return _core_property_node_api.v1.remove(node);
}

core_property_node_result_t core_property_node_attr_remove(
    const core_property_node_t *parent_node, const char *key)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(parent_node);
    log_assert(key);

    return _core_property_node_api.v1.attr_remove(parent_node, key);
}

core_property_node_result_t core_property_node_copy(
    core_property_node_t *dst_node, const core_property_node_t *src_node)
{
    log_assert(_core_property_node_api_is_valid());
    log_assert(dst_node);
    log_assert(src_node);

    return _core_property_node_api.v1.copy(dst_node, src_node);
}

#define LOG_MODULE "property-node-trace"

#include <string.h>

#include "core/property-node.h"

#include "iface-core/log.h"

static core_property_node_api_t _core_property_node_trace_target_api;

static void _core_property_node_trace_log(
    const core_property_node_t *node, bt_core_log_message_t log_message)
{
    log_misc(">>> log(%p)", node);

    _core_property_node_trace_target_api.v1.log(node, log_message);

    log_misc("<<< log(%p)", node);
}

static core_property_node_result_t _core_property_node_trace_name_get(
    const core_property_node_t *node, char *name, size_t len)
{
    core_property_node_result_t result;

    log_misc(">>> name_get(%p, %p, %d)", node, name, len);

    result = _core_property_node_trace_target_api.v1.name_get(node, name, len);

    log_misc(
        "<<< name_get(%p, %s): %s",
        node,
        name,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t
_core_property_node_trace_size(const core_property_node_t *node, size_t *size)
{
    core_property_node_result_t result;

    log_misc(">>> size(%p)", node);

    result = _core_property_node_trace_target_api.v1.size(node, size);

    log_misc(
        "<<< size(%p, %d): %s",
        node,
        *size,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_search(
    const core_property_node_t *node,
    const char *path,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(">>> search(%p, %s, %p)", node, path, node_out);

    result =
        _core_property_node_trace_target_api.v1.search(node, path, node_out);

    log_misc(
        "<<< search(%p, %s): %s",
        node,
        path,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_next_result_search(
    const core_property_node_t *node, core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(">>> next_result_search(%p, %p)", node, node_out);

    result = _core_property_node_trace_target_api.v1.next_result_search(
        node, node_out);

    log_misc(
        "<<< next_result_search(%p): %s",
        node,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_child_get(
    const core_property_node_t *node, core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(">>> child_get(%p, %p)", node, node_out);

    result = _core_property_node_trace_target_api.v1.child_get(node, node_out);

    log_misc(
        "<<< child_get(%p): %s",
        node,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_next_sibling_get(
    const core_property_node_t *node, core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(">>> next_sibling_get(%p, %p)", node, node_out);

    result = _core_property_node_trace_target_api.v1.next_sibling_get(
        node, node_out);

    log_misc(
        "<<< next_sibling_get(%p): %s",
        node,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_void_create(
    const core_property_node_t *parent_node,
    const char *key,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(">>> void_create(%p, %s, %p)", parent_node, key, node_out);

    result = _core_property_node_trace_target_api.v1.void_create(
        parent_node, key, node_out);

    log_misc(
        "<<< void_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s8_create(
    const core_property_node_t *parent_node,
    const char *key,
    int8_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> s8_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.s8_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< s8_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u8_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint8_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> u8_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.u8_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< u8_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s16_create(
    const core_property_node_t *parent_node,
    const char *key,
    int16_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> s16_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.s16_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< s16_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u16_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint16_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> u16_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.u16_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< u16_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s32_create(
    const core_property_node_t *parent_node,
    const char *key,
    int32_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> s32_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.s32_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< s32_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u32_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> u32_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.u32_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< u32_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s64_create(
    const core_property_node_t *parent_node,
    const char *key,
    int64_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> s64_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.s64_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< s64_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u64_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint64_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> u64_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.u64_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< u64_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_bin_create(
    const core_property_node_t *parent_node,
    const char *key,
    void *data,
    size_t len,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> bin_create(%p, %s, %p, %d, %p)",
        parent_node,
        key,
        data,
        len,
        node_out);

    result = _core_property_node_trace_target_api.v1.bin_create(
        parent_node, key, data, len, node_out);

    log_misc(
        "<<< bin_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_str_create(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> str_create(%p, %s, %s, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.str_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< str_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_ipv4_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> ipv4_create(%p, %s, %X, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.ipv4_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< ipv4_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_float_create(
    const core_property_node_t *parent_node,
    const char *key,
    float value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> float_create(%p, %s, %f, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.float_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< float_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_double_create(
    const core_property_node_t *parent_node,
    const char *key,
    double value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> double_create(%p, %s, %f, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.double_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< double_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_attr_create(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> attr_create(%p, %s, %s, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.attr_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< attr_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_bool_create(
    const core_property_node_t *parent_node,
    const char *key,
    bool value,
    core_property_node_t *node_out)
{
    core_property_node_result_t result;

    log_misc(
        ">>> bool_create(%p, %s, %d, %p)", parent_node, key, value, node_out);

    result = _core_property_node_trace_target_api.v1.bool_create(
        parent_node, key, value, node_out);

    log_misc(
        "<<< bool_create(%p, %s): %s",
        parent_node,
        key,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s8_read(
    const core_property_node_t *parent_node, int8_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> s8_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.s8_read(parent_node, value);

    log_misc(
        "<<< s8_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u8_read(
    const core_property_node_t *parent_node, uint8_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> u8_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.u8_read(parent_node, value);

    log_misc(
        "<<< u8_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s16_read(
    const core_property_node_t *parent_node, int16_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> s16_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.s16_read(parent_node, value);

    log_misc(
        "<<< s16_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u16_read(
    const core_property_node_t *parent_node, uint16_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> u16_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.u16_read(parent_node, value);

    log_misc(
        "<<< u16_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s32_read(
    const core_property_node_t *parent_node, int32_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> s32_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.s32_read(parent_node, value);

    log_misc(
        "<<< s32_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u32_read(
    const core_property_node_t *parent_node, uint32_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> u32_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.u32_read(parent_node, value);

    log_misc(
        "<<< u32_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_s64_read(
    const core_property_node_t *parent_node, int64_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> s64_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.s64_read(parent_node, value);

    log_misc(
        "<<< s64_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_u64_read(
    const core_property_node_t *parent_node, uint64_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> u64_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.u64_read(parent_node, value);

    log_misc(
        "<<< u64_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_bin_read(
    const core_property_node_t *parent_node, void *value, size_t len)
{
    core_property_node_result_t result;

    log_misc(">>> bin_read(%p, %p, %d)", parent_node, value, len);

    result = _core_property_node_trace_target_api.v1.bin_read(
        parent_node, value, len);

    log_misc(
        "<<< bin_read(%p): %s",
        parent_node,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_str_read(
    const core_property_node_t *parent_node, char *value, size_t len)
{
    core_property_node_result_t result;

    log_misc(">>> str_read(%p, %p, %d)", parent_node, value, len);

    result = _core_property_node_trace_target_api.v1.str_read(
        parent_node, value, len);

    log_misc(
        "<<< str_read(%p, %s): %s",
        parent_node,
        value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_ipv4_read(
    const core_property_node_t *parent_node, uint32_t *value)
{
    core_property_node_result_t result;

    log_misc(">>> ipv4_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.ipv4_read(parent_node, value);

    log_misc(
        "<<< ipv4_read(%p, %X): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_float_read(
    const core_property_node_t *parent_node, float *value)
{
    core_property_node_result_t result;

    log_misc(">>> float_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.float_read(parent_node, value);

    log_misc(
        "<<< float_read(%p, %f): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_double_read(
    const core_property_node_t *parent_node, double *value)
{
    core_property_node_result_t result;

    log_misc(">>> double_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.double_read(parent_node, value);

    log_misc(
        "<<< double_read(%p, %f): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_attr_read(
    const core_property_node_t *parent_node, char *value, size_t len)
{
    core_property_node_result_t result;

    log_misc(">>> attr_read(%p, %p, %d)", parent_node, value, len);

    result = _core_property_node_trace_target_api.v1.attr_read(
        parent_node, value, len);

    log_misc(
        "<<< attr_read(%p, %s): %s",
        parent_node,
        value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_bool_read(
    const core_property_node_t *parent_node, bool *value)
{
    core_property_node_result_t result;

    log_misc(">>> bool_read(%p)", parent_node);

    result =
        _core_property_node_trace_target_api.v1.bool_read(parent_node, value);

    log_misc(
        "<<< bool_read(%p, %d): %s",
        parent_node,
        *value,
        core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t
_core_property_node_trace_remove(const core_property_node_t *node)
{
    core_property_node_result_t result;

    log_misc(">>> remove(%p)", node);

    result = _core_property_node_trace_target_api.v1.remove(node);

    log_misc(
        "<<< remove(%p): %s", node, core_property_node_result_to_str(result));

    return result;
}

static core_property_node_result_t _core_property_node_trace_copy(
    core_property_node_t *dst_node, const core_property_node_t *src_node)
{
    core_property_node_result_t result;

    log_misc(">>> copy(%p, %p)", dst_node, src_node);

    result = _core_property_node_trace_target_api.v1.copy(dst_node, src_node);

    log_misc(
        "<<< copy(%p, %p): %s",
        dst_node,
        src_node,
        core_property_node_result_to_str(result));

    return result;
}

void core_property_node_trace_target_api_set(
    const core_property_node_api_t *target_api)
{
    log_assert(target_api);

    memcpy(
        &_core_property_node_trace_target_api,
        target_api,
        sizeof(core_property_node_api_t));
}

void core_property_node_trace_core_api_get(core_property_node_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.log = _core_property_node_trace_log;
    api->v1.name_get = _core_property_node_trace_name_get;
    api->v1.size = _core_property_node_trace_size;
    api->v1.search = _core_property_node_trace_search;
    api->v1.next_result_search = _core_property_node_trace_next_result_search;
    api->v1.child_get = _core_property_node_trace_child_get;
    api->v1.next_sibling_get = _core_property_node_trace_next_sibling_get;
    api->v1.void_create = _core_property_node_trace_void_create;
    api->v1.s8_create = _core_property_node_trace_s8_create;
    api->v1.u8_create = _core_property_node_trace_u8_create;
    api->v1.s16_create = _core_property_node_trace_s16_create;
    api->v1.u16_create = _core_property_node_trace_u16_create;
    api->v1.s32_create = _core_property_node_trace_s32_create;
    api->v1.u32_create = _core_property_node_trace_u32_create;
    api->v1.s64_create = _core_property_node_trace_s64_create;
    api->v1.u64_create = _core_property_node_trace_u64_create;
    api->v1.bin_create = _core_property_node_trace_bin_create;
    api->v1.str_create = _core_property_node_trace_str_create;
    api->v1.ipv4_create = _core_property_node_trace_ipv4_create;
    api->v1.float_create = _core_property_node_trace_float_create;
    api->v1.double_create = _core_property_node_trace_double_create;
    api->v1.attr_create = _core_property_node_trace_attr_create;
    api->v1.bool_create = _core_property_node_trace_bool_create;
    api->v1.s8_read = _core_property_node_trace_s8_read;
    api->v1.u8_read = _core_property_node_trace_u8_read;
    api->v1.s16_read = _core_property_node_trace_s16_read;
    api->v1.u16_read = _core_property_node_trace_u16_read;
    api->v1.s32_read = _core_property_node_trace_s32_read;
    api->v1.u32_read = _core_property_node_trace_u32_read;
    api->v1.s64_read = _core_property_node_trace_s64_read;
    api->v1.u64_read = _core_property_node_trace_u64_read;
    api->v1.bin_read = _core_property_node_trace_bin_read;
    api->v1.str_read = _core_property_node_trace_str_read;
    api->v1.ipv4_read = _core_property_node_trace_ipv4_read;
    api->v1.float_read = _core_property_node_trace_float_read;
    api->v1.double_read = _core_property_node_trace_double_read;
    api->v1.attr_read = _core_property_node_trace_attr_read;
    api->v1.bool_read = _core_property_node_trace_bool_read;
    api->v1.remove = _core_property_node_trace_remove;
    api->v1.copy = _core_property_node_trace_copy;
}
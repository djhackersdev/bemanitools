#define LOG_MODULE "core-config-property-node"

#include "api/core/config.h"

#include "main/core/config-property-node.h"
#include "main/core/property-node.h"
#include "main/core/property.h"

#include "iface-core/log.h"

#include "util/mem.h"

typedef struct core_config_property_node {
    const core_property_node_t *node;
} core_config_property_node_t;

static bt_core_config_result_t
_core_config_property_node_result_map(core_property_node_result_t result)
{
    switch (result) {
        case CORE_PROPERTY_NODE_RESULT_SUCCESS:
            return BT_CORE_CONFIG_RESULT_SUCCESS;
        case CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL:
            return BT_CORE_CONFIG_RESULT_ERROR_INTERNAL;
        case CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND:
            return BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND;
        default:
            return BT_CORE_CONFIG_RESULT_ERROR_INTERNAL;
    }
}

static bt_core_config_result_t _core_config_property_node_s8_get(
    const bt_core_config_t *config_, const char *path, int8_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s8_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_u8_get(
    const bt_core_config_t *config_, const char *path, uint8_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u8_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_s16_get(
    const bt_core_config_t *config_, const char *path, int16_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s16_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_u16_get(
    const bt_core_config_t *config_, const char *path, uint16_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u16_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_s32_get(
    const bt_core_config_t *config_, const char *path, int32_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s32_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_u32_get(
    const bt_core_config_t *config_, const char *path, uint32_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u32_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_s64_get(
    const bt_core_config_t *config_, const char *path, int64_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s64_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_u64_get(
    const bt_core_config_t *config_, const char *path, uint64_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u64_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_float_get(
    const bt_core_config_t *config_, const char *path, float *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_float_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_double_get(
    const bt_core_config_t *config_, const char *path, double *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_double_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_bool_get(
    const bt_core_config_t *config_, const char *path, bool *value)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_bool_read(&node, value);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_bin_get(
    const bt_core_config_t *config_, const char *path, void *value, size_t len)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_bin_read(&node, value, len);

    return _core_config_property_node_result_map(result);
}

static bt_core_config_result_t _core_config_property_node_str_get(
    const bt_core_config_t *config_, const char *path, char *value, size_t len)
{
    const core_config_property_node_t *config;
    core_property_node_t node;
    core_property_node_result_t result;

    config = (const core_config_property_node_t *) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_str_read(&node, value, len);

    return _core_config_property_node_result_map(result);
}

static void _core_config_property_node_core_api_get(bt_core_config_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.s8_get = _core_config_property_node_s8_get;
    api->v1.u8_get = _core_config_property_node_u8_get;
    api->v1.s16_get = _core_config_property_node_s16_get;
    api->v1.u16_get = _core_config_property_node_u16_get;
    api->v1.s32_get = _core_config_property_node_s32_get;
    api->v1.u32_get = _core_config_property_node_u32_get;
    api->v1.s64_get = _core_config_property_node_s64_get;
    api->v1.u64_get = _core_config_property_node_u64_get;
    api->v1.float_get = _core_config_property_node_float_get;
    api->v1.double_get = _core_config_property_node_double_get;
    api->v1.bool_get = _core_config_property_node_bool_get;
    api->v1.bin_get = _core_config_property_node_bin_get;
    api->v1.str_get = _core_config_property_node_str_get;
}

void core_config_property_node_core_api_set()
{
    bt_core_config_api_t api;

    _core_config_property_node_core_api_get(&api);
    bt_core_config_api_set(&api);
}

void core_config_property_node_init(
    const core_property_node_t *node, bt_core_config_t **config_)
{
    core_config_property_node_t *config;

    log_assert(node);
    log_assert(config_);

    config = xmalloc(sizeof(core_config_property_node_t));

    config->node = node;

    *config_ = (bt_core_config_t *) config;
}

void core_config_property_node_free(bt_core_config_t **config_)
{
    core_config_property_node_t **config;

    log_assert(config_);

    config = (core_config_property_node_t **) config_;

    free(*config);
    *config = NULL;
}
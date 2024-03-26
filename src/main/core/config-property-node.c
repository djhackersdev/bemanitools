#define LOG_MODULE "core-property-node"

#include "core/config-property-node.h"
#include "core/config.h"
#include "core/log.h"
#include "core/property-node.h"
#include "core/property.h"

#include "util/mem.h"

struct core_config_property_node {
    struct core_property_node *node;
};

static core_config_result_t _core_config_property_result_map(core_property_result_t result);
static core_config_result_t core_config_property_s8_get(const core_config_t *config, const char *path, int8_t *value);
static core_config_result_t core_config_property_u8_get(const core_config_t *config, const char *path, uint8_t *value);
static core_config_result_t core_config_property_s16_get(const core_config_t *config, const char *path, int16_t *value);
static core_config_result_t core_config_property_u16_get(const core_config_t *config, const char *path, uint16_t *value);
static core_config_result_t core_config_property_s32_get(const core_config_t *config, const char *path, int32_t *value);
static core_config_result_t core_config_property_u32_get(const core_config_t *config, const char *path, uint32_t *value);
static core_config_result_t core_config_property_s64_get(const core_config_t *config, const char *path, int64_t *value);
static core_config_result_t core_config_property_u64_get(const core_config_t *config, const char *path, uint64_t *value);
static core_config_result_t core_config_property_float_get(const core_config_t *config, const char *path, float *value);
static core_config_result_t core_config_property_double_get(const core_config_t *config, const char *path, double *value);
static core_config_result_t core_config_property_bool_get(const core_config_t *config, const char *path, bool *value);
static core_config_result_t core_config_property_bin_get(const core_config_t *config, const char *path, void *value, size_t len);
static core_config_result_t core_config_property_str_get(const core_config_t *config, const char *path, char *value, size_t len);

static core_config_impl_t _core_config_property_impl = {
    .s8_get = core_config_property_s8_get,
    .u8_get = core_config_property_u8_get,
    .s16_get = core_config_property_s16_get,
    .u16_get = core_config_property_u16_get,
    .s32_get = core_config_property_s32_get,
    .u32_get = core_config_property_u32_get,
    .s64_get = core_config_property_s64_get,
    .u64_get = core_config_property_u64_get,
    .float_get = core_config_property_float_get,
    .double_get = core_config_property_double_get,
    .bool_get = core_config_property_bool_get,
    .bin_get = core_config_property_bin_get,
    .str_get = core_config_property_str_get,
};

static core_config_result_t _core_config_property_node_result_map(core_property_node_result_t result)
{
    switch (result) {
        case CORE_PROPERTY_NODE_RESULT_SUCCESS:
            return CORE_CONFIG_RESULT_SUCCESS;
        case CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL:
            return CORE_CONFIG_RESULT_ERROR_INTERNAL;
        case CORE_PROPERTY_NODE_RESULT_ERROR_NODE_NOT_FOUND:
            return CORE_CONFIG_RESULT_ERROR_VALUE_NOT_FOUND;
        default:
            return CORE_CONFIG_RESULT_ERROR_INTERNAL;
    }
}

static core_config_result_t core_config_property_s8_get(const core_config_t *config_, const char *path, int8_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s8_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_u8_get(const core_config_t *config_, const char *path, uint8_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u8_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_s16_get(const core_config_t *config_, const char *path, int16_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s16_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_u16_get(const core_config_t *config_, const char *path, uint16_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u16_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_s32_get(const core_config_t *config_, const char *path, int32_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s32_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_u32_get(const core_config_t *config_, const char *path, uint32_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u32_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_s64_get(const core_config_t *config_, const char *path, int64_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_s64_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_u64_get(const core_config_t *config_, const char *path, uint64_t *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_u64_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_float_get(const core_config_t *config_, const char *path, float *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_float_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_double_get(const core_config_t *config_, const char *path, double *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_double_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_bool_get(const core_config_t *config_, const char *path, bool *value)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_bool_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_bin_get(const core_config_t *config_, const char *path, void *value, size_t len)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_bin_read(node, value);

    return _core_config_property_node_result_map(result);
}

static core_config_result_t core_config_property_str_get(const core_config_t *config_, const char *path, char *value, size_t len)
{
    const core_config_property_node_t *config;
    core_property_node_t *node;
    core_property_result_t result;

    config = (const core_config_property_node_t*) config_;

    result = core_property_node_search(config->node, path, &node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return _core_config_property_node_result_map(result);
    }

    result = core_property_node_str_read(node, value);

    return _core_config_property_node_result_map(result);
}

const core_config_impl_t *core_config_property_node_impl_get()
{
    return &_core_config_property_impl;
}

void core_config_property_node_init(struct core_property_node *node, core_config_property_node_t **config)
{
    log_assert(node);
    log_assert(config);

    *config = xmalloc(sizeof(core_config_property_node_t));

    (*config)->node = node;
}

void core_config_property_node_free(core_config_property_node_t **config)
{
    log_assert(config);

    free(*config);
}
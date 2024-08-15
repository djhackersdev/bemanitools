#define LOG_MODULE "bt-core-config"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "api/core/config.h"

#include "iface-core/log.h"

#define BT_CORE_CONFIG_ASSERT_IMPLEMENTED(func, name)              \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_core_config_api_t _bt_core_config_api;

static bool _bt_core_config_api_is_valid()
{
    return _bt_core_config_api.version > 0;
}

void bt_core_config_api_set(const bt_core_config_api_t *api)
{
    log_assert(api);

    if (_bt_core_config_api_is_valid()) {
        log_info("Re-initialize");
    }

    if (api->version == 1) {
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.s8_get, s8_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.u8_get, u8_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.s16_get, s16_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.u16_get, u16_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.s32_get, s32_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.u32_get, u32_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.s64_get, s64_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.u64_get, u64_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.float_get, float_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.double_get, double_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.bool_get, bool_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.bin_get, bin_get);
        BT_CORE_CONFIG_ASSERT_IMPLEMENTED(api->v1.str_get, str_get);

        memcpy(&_bt_core_config_api, api, sizeof(bt_core_config_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_core_config_api_get(bt_core_config_api_t *api)
{
    log_assert(api);
    log_assert(_bt_core_config_api_is_valid());

    memcpy(api, &_bt_core_config_api, sizeof(bt_core_config_api_t));
}

void bt_core_config_api_clear()
{
    log_assert(_bt_core_config_api_is_valid());

    memset(&_bt_core_config_api, 0, sizeof(bt_core_config_api_t));

    log_misc("api cleared");
}

const char *bt_core_config_result_to_str(bt_core_config_result_t result)
{
    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return "Success";
        case BT_CORE_CONFIG_RESULT_ERROR_INTERNAL:
            return "Internal";
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return "Value not found";
        case BT_CORE_CONFIG_RESULT_TYPE_MISMATCH:
            return "Type mismatch";
        default:
            return "Undefined error";
    }
}

void br_core_config_fatal_on_error(bt_core_config_result_t result)
{
    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return;
        case BT_CORE_CONFIG_RESULT_ERROR_INTERNAL:
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
        case BT_CORE_CONFIG_RESULT_TYPE_MISMATCH:
        default:
            log_fatal(
                "Operation on config failed: %s",
                bt_core_config_result_to_str(result));
    }
}

bt_core_config_result_t bt_core_config_s8_get(
    const bt_core_config_t *config, const char *path, int8_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.s8_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u8_get(
    const bt_core_config_t *config, const char *path, uint8_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.u8_get(config, path, value);
}

bt_core_config_result_t bt_core_config_s16_get(
    const bt_core_config_t *config, const char *path, int16_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.s16_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u16_get(
    const bt_core_config_t *config, const char *path, uint16_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.u16_get(config, path, value);
}

bt_core_config_result_t bt_core_config_s32_get(
    const bt_core_config_t *config, const char *path, int32_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.s32_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u32_get(
    const bt_core_config_t *config, const char *path, uint32_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.u32_get(config, path, value);
}

bt_core_config_result_t bt_core_config_s64_get(
    const bt_core_config_t *config, const char *path, int64_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.s64_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u64_get(
    const bt_core_config_t *config, const char *path, uint64_t *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.u64_get(config, path, value);
}

bt_core_config_result_t bt_core_config_float_get(
    const bt_core_config_t *config, const char *path, float *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.float_get(config, path, value);
}

bt_core_config_result_t bt_core_config_double_get(
    const bt_core_config_t *config, const char *path, double *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.double_get(config, path, value);
}

bt_core_config_result_t bt_core_config_bool_get(
    const bt_core_config_t *config, const char *path, bool *value)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _bt_core_config_api.v1.bool_get(config, path, value);
}

bt_core_config_result_t bt_core_config_bin_get(
    const bt_core_config_t *config, const char *path, void *value, size_t len)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);
    log_assert(len > 0);

    return _bt_core_config_api.v1.bin_get(config, path, value, len);
}

bt_core_config_result_t bt_core_config_str_get(
    const bt_core_config_t *config, const char *path, char *value, size_t len)
{
    log_assert(_bt_core_config_api_is_valid());
    log_assert(config);
    log_assert(path);
    log_assert(value);
    log_assert(len > 0);

    return _bt_core_config_api.v1.str_get(config, path, value, len);
}
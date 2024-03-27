#define LOG_MODULE "btsdk-core-config"

#include "btsdk/core/config.h"
#include "btsdk/core/log.h"

static bt_core_config_impl_t *_bt_core_config_impl;

void bt_core_config_impl_set(const bt_core_config_impl_t *impl)
{
    log_assert(impl);

    if (_bt_core_config_impl) {
        log_warning("Re-initialize");
    }

    memcpy(_bt_core_config_impl, impl, sizeof(bt_core_config_impl_t));
}

const char *bt_core_config_result_to_str(bt_core_config_result_t result)
{
    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return "Success";
        case BT_CORE_CONFIG_RESULT_ERROR_INTERNAL:
            return "Internal";
        case BT_CORE_CONFIG_RESULT_ERROR_VALUE_NOT_FOUND:
            return "Value for given key not found";
        case BT_CORE_CONFIG_RESULT_ERROR_TYPE_MISMATCH:
            return "Type mismatch on value for given key";
        default:
            return "Undefined error";
    }
}

void bt_core_config_fatal_on_error(bt_core_config_result_t result)
{
    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return;
        case BT_CORE_CONFIG_RESULT_ERROR_VALUE_NOT_FOUND:
        case BT_CORE_CONFIG_RESULT_ERROR_INTERNAL:
        case BT_CORE_CONFIG_RESULT_ERROR_TYPE_MISMATCH:
        default:
            log_fatal("Operation on config failed: %s", bt_core_config_result_to_str(result));
    }
}

bt_core_config_result_t bt_core_config_s8_get(const bt_core_config_t *config, const char *path, int8_t *value)
{
    return _bt_core_config_impl->s8_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u8_get(const bt_core_config_t *config, const char *path, uint8_t *value)
{
    return _bt_core_config_impl->u8_get(config, path, value);
}

bt_core_config_result_t bt_core_config_s16_get(const bt_core_config_t *config, const char *path, int16_t *value)
{
    return _bt_core_config_impl->s16_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u16_get(const bt_core_config_t *config, const char *path, uint16_t *value
{
    return _bt_core_config_impl->u16_get(config, path, value);
}

bt_core_config_result_t bt_core_config_s32_get(const bt_core_config_t *config, const char *path, int32_t *value)
{
    return _bt_core_config_impl->s32_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u32_get(const bt_core_config_t *config, const char *path, uint32_t *value
{
    return _bt_core_config_impl->u32_get(config, path, value);
}

bt_core_config_result_t bt_core_config_s64_get(const bt_core_config_t *config, const char *path, int64_t *value)
{
    return _bt_core_config_impl->s64_get(config, path, value);
}

bt_core_config_result_t bt_core_config_u64_get(const bt_core_config_t *config, const char *path, uint64_t *value
{
    return _bt_core_config_impl->u64_get(config, path, value);
}

bt_core_config_result_t bt_core_config_float_get(const bt_core_config_t *config, const char *path, float *value)
{
    return _bt_core_config_impl->float_get(config, path, value);
}

bt_core_config_result_t bt_core_config_double_get(const bt_core_config_t *config, const char *path, double *value)
{
    return _bt_core_config_impl->double_get(config, path, value);    
}

bt_core_config_result_t bt_core_config_bool_get(const bt_core_config_t *config, const char *path, bool *value)
{
    return _bt_core_config_impl->bool_get(config, path, value);   
}

bt_core_config_result_t bt_core_config_bin_get(const bt_core_config_t *config, const char *path, void *value, size_t len)
{
    return _bt_core_config_impl->bin_get(config, path, value, len);
}

bt_core_config_result_t bt_core_config_str_get(const bt_core_config_t *config, const char *path, char *value, size_t len)
{
    return _bt_core_config_impl->str_get(config, path, value, len);
}

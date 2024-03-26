#define LOG_MODULE "core-config"

#include "core/config.h"
#include "core/log.h"

static core_config_impl_t *_core_config_impl;

void core_config_impl_set(const core_config_impl_t *impl)
{
    log_assert(impl);

    if (_core_config_impl) {
        log_warning("Re-initialize");
    }

    _core_config_impl = impl;
}

const core_config_impl_t *core_config_impl_get()
{
    log_assert(_core_config_impl);

    return _core_config_impl;
}

const char *core_config_result_to_str(core_config_result_t result)
{
    switch (result) {
        case CORE_CONFIG_RESULT_SUCCESS:
            return "Success";
        case CORE_CONFIG_RESULT_ERROR_INTERNAL:
            return "Internal";
        case CORE_CONFIG_RESULT_ERROR_VALUE_NOT_FOUND:
            return "Value for given key not found";
        case CORE_CONFIG_RESULT_ERROR_TYPE_MISMATCH:
            return "Type mismatch on value for given key";
        default:
            return "Undefined error";
    }
}

void core_config_fatal_on_error(core_config_result_t result)
{
    switch (result) {
        case CORE_CONFIG_RESULT_SUCCESS:
            return;
        case CORE_CONFIG_RESULT_ERROR_VALUE_NOT_FOUND:
        case CORE_CONFIG_RESULT_ERROR_INTERNAL:
        case CORE_CONFIG_RESULT_ERROR_TYPE_MISMATCH:
        default:
            log_fatal("Operation on config failed: %s", core_config_result_to_str(result));
    }
}

core_config_result_t core_config_s8_get(const core_config_t *config, const char *path, int8_t *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->s8_get(config, path, value);
}

core_config_result_t core_config_u8_get(const core_config_t *config, const char *path, uint8_t *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->u8_get(config, path, value);
}

core_config_result_t core_config_s16_get(const core_config_t *config, const char *path, int16_t *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->s16_get(config, path, value);
}

core_config_result_t core_config_u16_get(const core_config_t *config, const char *path, uint16_t *value
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->u16_get(config, path, value);
}

core_config_result_t core_config_s32_get(const core_config_t *config, const char *path, int32_t *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->s32_get(config, path, value);
}

core_config_result_t core_config_u32_get(const core_config_t *config, const char *path, uint32_t *value
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->u32_get(config, path, value);
}

core_config_result_t core_config_s64_get(const core_config_t *config, const char *path, int64_t *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->s64_get(config, path, value);
}

core_config_result_t core_config_u64_get(const core_config_t *config, const char *path, uint64_t *value
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->u64_get(config, path, value);
}

core_config_result_t core_config_float_get(const core_config_t *config, const char *path, float *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->float_get(config, path, value);
}

core_config_result_t core_config_double_get(const core_config_t *config, const char *path, double *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->double_get(config, path, value);    
}

core_config_result_t core_config_bool_get(const core_config_t *config, const char *path, bool *value)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);

    return _core_config_impl->bool_get(config, path, value);   
}

core_config_result_t core_config_bin_get(const core_config_t *config, const char *path, void *value, size_t len)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->bin_get(config, path, value, len);
}

core_config_result_t core_config_str_get(const core_config_t *config, const char *path, char *value, size_t len)
{
    log_assert(_core_config_impl);
    log_assert(config);
    log_assert(path);
    log_assert(value);
    
    return _core_config_impl->str_get(config, path, value, len);
}

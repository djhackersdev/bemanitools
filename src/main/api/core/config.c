#include "btapi/core/config.h"

#include "core/config.h"

static bt_core_config_result_t _api_core_config_s8_get_t(const bt_core_config_t *config, const char *path, int8_t *value)
{
    return (bt_core_config_result_t) core_config_s8_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_u8_get_t(const bt_core_config_t *config, const char *path, uint8_t *value)
{
    return (bt_core_config_result_t) core_config_u8_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_s16_get_t(const bt_core_config_t *config, const char *path, int16_t *value)
{
    return (bt_core_config_result_t) core_config_s16_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_u16_get_t(const bt_core_config_t *config, const char *path, uint16_t *value)
{
    return (bt_core_config_result_t) core_config_u16_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_s32_get_t(const bt_core_config_t *config, const char *path, int32_t *value)
{
    return (bt_core_config_result_t) core_config_s32_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_u32_get_t(const bt_core_config_t *config, const char *path, uint32_t *value)
{
    return (bt_core_config_result_t) core_config_u32_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_s64_get_t(const bt_core_config_t *config, const char *path, int64_t *value)
{
    return (bt_core_config_result_t) core_config_s64_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_u64_get_t(const bt_core_config_t *config, const char *path, uint64_t *value)
{
    return (bt_core_config_result_t) core_config_u64_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_float_get_t(const bt_core_config_t *config, const char *path, float *value)
{
    return (bt_core_config_result_t) core_config_float_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_double_get_t(const bt_core_config_t *config, const char *path, double *value)
{
    return (bt_core_config_result_t) core_config_double_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_bool_get_t(const bt_core_config_t *config, const char *path, bool *value)
{
    return (bt_core_config_result_t) core_config_bool_get((const core_config_t*) config, path, value);
}

static bt_core_config_result_t _api_core_config_bin_get_t(const bt_core_config_t *config, const char *path, void *value, size_t len)
{
    return (bt_core_config_result_t) core_config_bin_get((const core_config_t*) config, path, value, len);
}

static bt_core_config_result_t _api_core_config_str_get_t(const bt_core_config_t *config, const char *path, char *value, size_t len)
{
    return (bt_core_config_result_t) core_config_str_get((const core_config_t*) config, path, value, len);
}

void api_core_config_impl_get(bt_core_config_impl_t *impl)
{
    log_assert(impl);

    impl->s8_get = _api_core_config_u8_get_t;
    impl->u8_get = _api_core_config_s8_get_t;
    impl->s16_get = _api_core_config_s16_get_t;
    impl->u16_get = _api_core_config_u16_get_t;
    impl->s32_get = _api_core_config_s32_get_t;
    impl->u32_get = _api_core_config_u32_get_t;
    impl->s64_get = _api_core_config_s64_get_t;
    impl->u64_get = _api_core_config_u64_get_t;
    impl->float_get = _api_core_config_float_get_t;
    impl->double_get = _api_core_config_double_get_t;
    impl->bool_get = _api_core_config_bool_get_t;
    impl->bin_get = _api_core_config_bin_get_t;
    impl->str_get = _api_core_config_str_get_t;
}
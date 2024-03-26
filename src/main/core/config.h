#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct core_config;
typedef struct core_config core_config_t;

enum core_config_result {
    CORE_CONFIG_RESULT_SUCCESS = 0,
    CORE_CONFIG_RESULT_ERROR_INTERNAL = 1,
    CORE_CONFIG_RESULT_VALUE_NOT_FOUND = 2,
    CORE_CONFIG_RESULT_TYPE_MISMATCH = 3,
};
typedef enum core_config_result core_config_result_t;

typedef core_config_result_t (*core_config_s8_get_t)(const core_config_t *config, const char *path, int8_t *value);
typedef core_config_result_t (*core_config_u8_get_t)(const core_config_t *config, const char *path, uint8_t *value);
typedef core_config_result_t (*core_config_s16_get_t)(const core_config_t *config, const char *path, int16_t *value);
typedef core_config_result_t (*core_config_u16_get_t)(const core_config_t *config, const char *path, uint16_t *value);
typedef core_config_result_t (*core_config_s32_get_t)(const core_config_t *config, const char *path, int32_t *value);
typedef core_config_result_t (*core_config_u32_get_t)(const core_config_t *config, const char *path, uint32_t *value);
typedef core_config_result_t (*core_config_s64_get_t)(const core_config_t *config, const char *path, int64_t *value);
typedef core_config_result_t (*core_config_u64_get_t)(const core_config_t *config, const char *path, uint64_t *value);
typedef core_config_result_t (*core_config_float_get_t)(const core_config_t *config, const char *path, float *value);
typedef core_config_result_t (*core_config_double_get_t)(const core_config_t *config, const char *path, double *value);
typedef core_config_result_t (*core_config_bool_get_t)(const core_config_t *config, const char *path, bool *value);
typedef core_config_result_t (*core_config_bin_get_t)(const core_config_t *config, const char *path, void *value, size_t len);
typedef core_config_result_t (*core_config_str_get_t)(const core_config_t *config, const char *path, char *value, size_t len);

struct core_config_impl {
    core_config_s8_get_t s8_get;
    core_config_u8_get_t u8_get;
    core_config_s16_get_t s16_get;
    core_config_u16_get_t u16_get;
    core_config_s32_get_t s32_get;
    core_config_u32_get_t u32_get;
    core_config_s64_get_t s64_get;
    core_config_u64_get_t u64_get;
    core_config_float_get_t float_get;
    core_config_double_get_t double_get;
    core_config_bool_get_t bool_get;
    core_config_bin_get_t bin_get;
    core_config_str_get_t str_get;
};

typedef struct core_config_impl core_config_impl_t;

void core_config_impl_set(const core_config_impl_t *impl);
const core_config_impl_t *core_config_impl_get(); 

const char *core_config_result_to_str(core_config_result_t result);
void core_config_fatal_on_error(core_config_result_t result);

core_config_result_t core_config_s8_get(const core_config_t *config, const char *path, int8_t *value);
core_config_result_t core_config_u8_get(const core_config_t *config, const char *path, uint8_t *value);
core_config_result_t core_config_s16_get(const core_config_t *config, const char *path, int16_t *value);
core_config_result_t core_config_u16_get(const core_config_t *config, const char *path, uint16_t *value);
core_config_result_t core_config_s32_get(const core_config_t *config, const char *path, int32_t *value);
core_config_result_t core_config_u32_get(const core_config_t *config, const char *path, uint32_t *value);
core_config_result_t core_config_s64_get(const core_config_t *config, const char *path, int64_t *value);
core_config_result_t core_config_u64_get(const core_config_t *config, const char *path, uint64_t *value);
core_config_result_t core_config_float_get(const core_config_t *config, const char *path, float *value);
core_config_result_t core_config_double_get(const core_config_t *config, const char *path, double *value);
core_config_result_t core_config_bool_get(const core_config_t *config, const char *path, bool *value);
core_config_result_t core_config_bin_get(const core_config_t *config, const char *path, void *value, size_t len);
core_config_result_t core_config_str_get(const core_config_t *config, const char *path, char *value, size_t len);

#endif
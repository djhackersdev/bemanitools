#ifndef BT_API_CORE_CONFIG_H
#define BT_API_CORE_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct bt_core_config bt_core_config_t;

typedef enum bt_core_config_result {
    BT_CORE_CONFIG_RESULT_SUCCESS = 0,
    BT_CORE_CONFIG_RESULT_ERROR_INTERNAL = 1,
    BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND = 2,
    BT_CORE_CONFIG_RESULT_TYPE_MISMATCH = 3,
} bt_core_config_result_t;

typedef bt_core_config_result_t (*bt_core_config_s8_get_t)(const bt_core_config_t *config, const char *path, int8_t *value);
typedef bt_core_config_result_t (*bt_core_config_u8_get_t)(const bt_core_config_t *config, const char *path, uint8_t *value);
typedef bt_core_config_result_t (*bt_core_config_s16_get_t)(const bt_core_config_t *config, const char *path, int16_t *value);
typedef bt_core_config_result_t (*bt_core_config_u16_get_t)(const bt_core_config_t *config, const char *path, uint16_t *value);
typedef bt_core_config_result_t (*bt_core_config_s32_get_t)(const bt_core_config_t *config, const char *path, int32_t *value);
typedef bt_core_config_result_t (*bt_core_config_u32_get_t)(const bt_core_config_t *config, const char *path, uint32_t *value);
typedef bt_core_config_result_t (*bt_core_config_s64_get_t)(const bt_core_config_t *config, const char *path, int64_t *value);
typedef bt_core_config_result_t (*bt_core_config_u64_get_t)(const bt_core_config_t *config, const char *path, uint64_t *value);
typedef bt_core_config_result_t (*bt_core_config_float_get_t)(const bt_core_config_t *config, const char *path, float *value);
typedef bt_core_config_result_t (*bt_core_config_double_get_t)(const bt_core_config_t *config, const char *path, double *value);
typedef bt_core_config_result_t (*bt_core_config_bool_get_t)(const bt_core_config_t *config, const char *path, bool *value);
typedef bt_core_config_result_t (*bt_core_config_bin_get_t)(const bt_core_config_t *config, const char *path, void *value, size_t len);
typedef bt_core_config_result_t (*bt_core_config_str_get_t)(const bt_core_config_t *config, const char *path, char *value, size_t len);

typedef struct bt_core_config_api {
    uint16_t version;

    struct {
        bt_core_config_s8_get_t s8_get;
        bt_core_config_u8_get_t u8_get;
        bt_core_config_s16_get_t s16_get;
        bt_core_config_u16_get_t u16_get;
        bt_core_config_s32_get_t s32_get;
        bt_core_config_u32_get_t u32_get;
        bt_core_config_s64_get_t s64_get;
        bt_core_config_u64_get_t u64_get;
        bt_core_config_float_get_t float_get;
        bt_core_config_double_get_t double_get;
        bt_core_config_bool_get_t bool_get;
        bt_core_config_bin_get_t bin_get;
        bt_core_config_str_get_t str_get;
    } v1;
} bt_core_config_api_t;

#endif
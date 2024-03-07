#ifndef BTAPI_CONFIG_H
#define BTAPI_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define BTAPI_CONFIG_FAIL_ON_ERROR(x) \
    while (0) { \
        btapi_config_result_t result = x; \
        switch (result) { \
            case BTAPI_CONFIG_RESULT_OK: \
            case BTAPI_CONFIG_RESULT_NOT_FOUND: \
                break; \
            default: \
                log_fatal("Operation on config failed: %s", btapi_config_result_to_str(result));  \
        } \
    }

enum btapi_config_result {
    BTAPI_CONFIG_RESULT_OK = 0,
    BTAPI_CONFIG_RESULT_NOT_FOUND = 1,
    BTAPI_CONFIG_RESULT_TYPE_ERROR = 2,
};

typedef enum btapi_config_result btapi_config_result_t;

struct btapi_config;
typedef struct btapi_config btapi_config_t;

const char *btapi_config_result_to_str(btapi_config_result_t result);

btapi_config_result_t btapi_config_s8_get(const btapi_config_t *config, const char *path, int8_t *value);
btapi_config_result_t btapi_config_u8_get(const btapi_config_t *config, const char *path, uint8_t *value);
btapi_config_result_t btapi_config_s16_get(const btapi_config_t *config, const char *path, int16_t *value);
btapi_config_result_t btapi_config_u16_get(const btapi_config_t *config, const char *path, uint16_t *value);
btapi_config_result_t btapi_config_s32_get(const btapi_config_t *config, const char *path, int32_t *value);
btapi_config_result_t btapi_config_u32_get(const btapi_config_t *config, const char *path, uint32_t *value);
btapi_config_result_t btapi_config_s64_get(const btapi_config_t *config, const char *path, int64_t *value);
btapi_config_result_t btapi_config_u64_get(const btapi_config_t *config, const char *path, uint64_t *value);
btapi_config_result_t btapi_config_float_get(const btapi_config_t *config, const char *path, float *value);
btapi_config_result_t btapi_config_double_get(const btapi_config_t *config, const char *path, double *value);
btapi_config_result_t btapi_config_bool_get(const btapi_config_t *config, const char *path, bool *value);
btapi_config_result_t btapi_config_bin_get(const btapi_config_t *config, const char *path, void *value, size_t len);
btapi_config_result_t btapi_config_str_get(const btapi_config_t *config, const char *path, char *value, size_t len);

#endif
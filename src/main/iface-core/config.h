#ifndef BT_CORE_CONFIG_H
#define BT_CORE_CONFIG_H

#include <stdint.h>

#include "api/core/config.h"

void bt_core_config_api_set(const bt_core_config_api_t *api);
void bt_core_config_api_get(bt_core_config_api_t *api);
void bt_core_config_api_clear();

const char *bt_core_config_result_to_str(bt_core_config_result_t result);
void bt_core_config_fatal_on_error(bt_core_config_result_t result, const char *path);

bt_core_config_result_t bt_core_config_s8_get(
    const bt_core_config_t *config, const char *path, int8_t *value);
bt_core_config_result_t bt_core_config_u8_get(
    const bt_core_config_t *config, const char *path, uint8_t *value);
bt_core_config_result_t bt_core_config_s16_get(
    const bt_core_config_t *config, const char *path, int16_t *value);
bt_core_config_result_t bt_core_config_u16_get(
    const bt_core_config_t *config, const char *path, uint16_t *value);
bt_core_config_result_t bt_core_config_s32_get(
    const bt_core_config_t *config, const char *path, int32_t *value);
bt_core_config_result_t bt_core_config_u32_get(
    const bt_core_config_t *config, const char *path, uint32_t *value);
bt_core_config_result_t bt_core_config_s64_get(
    const bt_core_config_t *config, const char *path, int64_t *value);
bt_core_config_result_t bt_core_config_u64_get(
    const bt_core_config_t *config, const char *path, uint64_t *value);
bt_core_config_result_t bt_core_config_float_get(
    const bt_core_config_t *config, const char *path, float *value);
bt_core_config_result_t bt_core_config_double_get(
    const bt_core_config_t *config, const char *path, double *value);
bt_core_config_result_t bt_core_config_bool_get(
    const bt_core_config_t *config, const char *path, bool *value);
bt_core_config_result_t bt_core_config_bin_get(
    const bt_core_config_t *config, const char *path, void *value, size_t len);
bt_core_config_result_t bt_core_config_str_get(
    const bt_core_config_t *config, const char *path, char *value, size_t len);

#endif
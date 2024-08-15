#ifndef CORE_CONFIG_EXT_H
#define CORE_CONFIG_EXT_H

#include "iface-core/config.h"

#include "util/net.h"

void bt_core_config_ext_s8_get(
    const bt_core_config_t *config, const char *path, int8_t *value);
void bt_core_config_ext_u8_get(
    const bt_core_config_t *config, const char *path, uint8_t *value);
void bt_core_config_ext_s16_get(
    const bt_core_config_t *config, const char *path, int16_t *value);
void bt_core_config_ext_u16_get(
    const bt_core_config_t *config, const char *path, uint16_t *value);
void bt_core_config_ext_s32_get(
    const bt_core_config_t *config, const char *path, int32_t *value);
void bt_core_config_ext_u32_get(
    const bt_core_config_t *config, const char *path, uint32_t *value);
void bt_core_config_ext_s64_get(
    const bt_core_config_t *config, const char *path, int64_t *value);
void bt_core_config_ext_u64_get(
    const bt_core_config_t *config, const char *path, uint64_t *value);
void bt_core_config_ext_float_get(
    const bt_core_config_t *config, const char *path, float *value);
void bt_core_config_ext_double_get(
    const bt_core_config_t *config, const char *path, double *value);
void bt_core_config_ext_bool_get(
    const bt_core_config_t *config, const char *path, bool *value);
void bt_core_config_ext_bin_get(
    const bt_core_config_t *config, const char *path, void *value, size_t len);
void bt_core_config_ext_str_get(
    const bt_core_config_t *config, const char *path, char *value, size_t len);
void bt_core_config_ext_net_addr_get(const bt_core_config_t *config, const char *path, struct net_addr *addr);

#endif
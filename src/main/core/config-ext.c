#include "iface-core/config.h"
#include "iface-core/log.h"

#include "util/net.h"

void bt_core_config_ext_s8_get(
    const bt_core_config_t *config, const char *path, int8_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s8_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_u8_get(
    const bt_core_config_t *config, const char *path, uint8_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u8_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_s16_get(
    const bt_core_config_t *config, const char *path, int16_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s16_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_u16_get(
    const bt_core_config_t *config, const char *path, uint16_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u16_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_s32_get(
    const bt_core_config_t *config, const char *path, int32_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s32_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_u32_get(
    const bt_core_config_t *config, const char *path, uint32_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u32_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_s64_get(
    const bt_core_config_t *config, const char *path, int64_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s64_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_u64_get(
    const bt_core_config_t *config, const char *path, uint64_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u64_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_float_get(
    const bt_core_config_t *config, const char *path, float *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_float_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_double_get(
    const bt_core_config_t *config, const char *path, double *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_double_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_bool_get(
    const bt_core_config_t *config, const char *path, bool *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_bool_get(config, path, value);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_bin_get(
    const bt_core_config_t *config, const char *path, void *value, size_t len)
{
    bt_core_config_result_t result;

    result = bt_core_config_bin_get(config, path, value, len);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_str_get(
    const bt_core_config_t *config, const char *path, char *value, size_t len)
{
    bt_core_config_result_t result;

    result = bt_core_config_str_get(config, path, value, len);
    bt_core_config_fatal_on_error(result, path);
}

void bt_core_config_ext_net_addr_get(const bt_core_config_t *config, const char *path, struct net_addr *addr)
{
    char buffer[1024];

    bt_core_config_ext_str_get(config, path, buffer, sizeof(buffer));

    if (!net_str_parse(buffer, addr)) {
        log_fatal("Cannot parse server address: %s", buffer);
    }
}

bool bt_core_config_ext_s8_optional_get(
    const bt_core_config_t *config, const char *path, int8_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s8_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    }
}

bool bt_core_config_ext_u8_optional_get(
    const bt_core_config_t *config, const char *path, uint8_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u8_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_s16_optional_get(
    const bt_core_config_t *config, const char *path, int16_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s16_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_u16_optional_get(
    const bt_core_config_t *config, const char *path, uint16_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u16_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_s32_optional_get(
    const bt_core_config_t *config, const char *path, int32_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s32_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_u32_optional_get(
    const bt_core_config_t *config, const char *path, uint32_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u32_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_s64_optional_get(
    const bt_core_config_t *config, const char *path, int64_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_s64_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_u64_optional_get(
    const bt_core_config_t *config, const char *path, uint64_t *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_u64_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_float_optional_get(
    const bt_core_config_t *config, const char *path, float *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_float_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_double_optional_get(
    const bt_core_config_t *config, const char *path, double *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_double_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_bool_optional_get(
    const bt_core_config_t *config, const char *path, bool *value)
{
    bt_core_config_result_t result;

    result = bt_core_config_bool_get(config, path, value);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_bin_optional_get(
    const bt_core_config_t *config, const char *path, void *value, size_t len)
{
    bt_core_config_result_t result;

    result = bt_core_config_bin_get(config, path, value, len);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_str_optional_get(
    const bt_core_config_t *config, const char *path, char *value, size_t len)
{
    bt_core_config_result_t result;

    result = bt_core_config_str_get(config, path, value, len);

    switch (result) {
        case BT_CORE_CONFIG_RESULT_SUCCESS:
            return true;
        case BT_CORE_CONFIG_RESULT_VALUE_NOT_FOUND:
            return false;
        default:
            bt_core_config_fatal_on_error(result, path);
            // Should not reach
            return false;
    } 
}

bool bt_core_config_ext_net_addr_optional_get(const bt_core_config_t *config, const char *path, struct net_addr *addr)
{
    char buffer[1024];
    bool result;

    result = bt_core_config_ext_str_optional_get(config, path, buffer, sizeof(buffer));

    if (!result) {
        return false;
    }

    if (!net_str_parse(buffer, addr)) {
        log_fatal("Cannot parse server address: %s", buffer);
    }

    return true;
}
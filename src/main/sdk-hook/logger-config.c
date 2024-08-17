#define LOG_MODULE "bt-hook-inject-logger-config"

#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "sdk-hook/logger-config.h"

#include "util/str.h"

static enum core_log_bt_log_level _bt_hook_logger_config_str_to_loglevel(const char *str)
{
    log_assert(str);

    if (str_eq(str, "off")) {
        return CORE_LOG_BT_LOG_LEVEL_OFF;
    } else if (str_eq(str, "fatal")) {
        return CORE_LOG_BT_LOG_LEVEL_FATAL;
    } else if (str_eq(str, "warning")) {
        return CORE_LOG_BT_LOG_LEVEL_WARNING;
    } else if (str_eq(str, "info")) {
        return CORE_LOG_BT_LOG_LEVEL_INFO;
    } else if (str_eq(str, "misc")) {
        return CORE_LOG_BT_LOG_LEVEL_MISC;
    } else {
        log_fatal("Invalid log level string in config: %s", str);
    }
}

void bt_hook_logger_config_init(bt_hook_logger_config_t *config)
{
    log_assert(config);

    memset(config, 0, sizeof(bt_hook_logger_config_t));
}

void bt_hook_logger_config_load(
    const core_property_node_t *node, bt_hook_logger_config_t *config)
{
    core_property_node_result_t result;
    char buffer[16];

    log_assert(node);
    log_assert(config);

    result = core_property_node_ext_bool_read(node, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(node, "level", buffer, sizeof(buffer));
    core_property_node_fatal_on_error(result);
    config->level = _bt_hook_logger_config_str_to_loglevel(buffer);
}

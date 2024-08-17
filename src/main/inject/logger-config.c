#define LOG_MODULE "inject-logger-config"

#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "inject/logger-config.h"

#include "util/str.h"

static enum core_log_bt_log_level _logger_config_str_to_loglevel(const char *str)
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

static enum core_log_sink_async_overflow_policy _logger_config_str_to_overflowpolicy(const char *str)
{
    log_assert(str);

    if (str_eq(str, "discard_new")) {
        return CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_DISCARD_NEW;
    } else if (str_eq(str, "block")) {
        return CORE_LOG_SINK_ASYNC_OVERFLOW_POLICY_BLOCK;
    } else {
        log_fatal("Invalid overflow policy string in config: %s", str);
    }
}

static void _logger_config_sink_async_load(const core_property_node_t *node, struct logger_sink_async_config *config)
{
    core_property_node_result_t result;
    core_property_node_t child;
    char buffer[16];

    log_assert(node);
    log_assert(config);

    result = core_property_node_search(node, "sinks/async", &child);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read(&child, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_u8_read(&child, "queue_length", &config->queue_length);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(&child, "overflow_policy", buffer, sizeof(buffer));
    core_property_node_fatal_on_error(result);
    config->overflow_policy = _logger_config_str_to_overflowpolicy(buffer);
}

static void _logger_config_sink_console_load(const core_property_node_t *node, struct logger_sink_console_config *config)
{
    core_property_node_result_t result;
    core_property_node_t child;

    log_assert(node);
    log_assert(config);

    result = core_property_node_search(node, "sinks/console", &child);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read(&child, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read(&child, "color", &config->color);
    core_property_node_fatal_on_error(result);
}

static void _logger_config_sink_file_load(const core_property_node_t *node, struct logger_sink_file_config *config)
{
    core_property_node_result_t result;
    core_property_node_t child;

    log_assert(node);
    log_assert(config);

    result = core_property_node_search(node, "sinks/file", &child);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read(&child, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(&child, "path", config->path, sizeof(config->path));
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read(&child, "append", &config->append);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read(&child, "rotate", &config->rotate);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_u8_read(&child, "max_rotations", &config->max_rotations);
    core_property_node_fatal_on_error(result);
}

void logger_config_init(logger_config_t *config)
{
    log_assert(config);

    memset(config, 0, sizeof(logger_config_t));
}

void logger_config_load(
    const core_property_node_t *node, logger_config_t *config)
{
    core_property_node_result_t result;
    char buffer[16];

    log_assert(node);
    log_assert(config);

    result = core_property_node_ext_bool_read(node, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(node, "level", buffer, sizeof(buffer));
    core_property_node_fatal_on_error(result);
    config->level = _logger_config_str_to_loglevel(buffer);

    result = core_property_node_ext_u32_read(node, "msg_buffer_size_bytes", &config->msg_buffer_size_bytes);
    core_property_node_fatal_on_error(result);

    _logger_config_sink_async_load(node, &config->sink_async);
    _logger_config_sink_console_load(node, &config->sink_console);
    _logger_config_sink_file_load(node, &config->sink_file);
}

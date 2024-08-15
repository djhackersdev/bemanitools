#define LOG_MODULE "inject-logger"

#include "core/log-bt.h"
#include "core/log-sink-async.h"
#include "core/log-sink-file.h"
#include "core/log-sink-list.h"
#include "core/log-sink-null.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"

#include "inject/logger-config.h"

static void _logger_null_sink_init()
{
    core_log_sink_t sink;

    core_log_sink_null_open(&sink);

    // Size doesn't matter (but must be valid)
    // logger is entirely disabled
    core_log_bt_init(1024, &sink);
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_OFF);
}

static bool _logger_sinks_create(
    const logger_config_t *config,
    core_log_sink_t *root_sink)
{
    core_log_sink_t target_sinks[2];
    core_log_sink_t list_sink;
    uint8_t target_sink_count;

    log_assert(config);
    log_assert(root_sink);

    target_sink_count = 0;

    // Fixed order to ensure logger's first sink to write to is
    // async and async sinks to console and file

    if (config->sink_console.enable) {
        core_log_sink_std_err_open(
            config->sink_console.color,
            &target_sinks[target_sink_count]);

        target_sink_count++;
    }

    if (config->sink_file.enable) {
        core_log_sink_file_open(
            config->sink_file.path,
            config->sink_file.append,
            config->sink_file.rotate,
            config->sink_file.max_rotations,
            &target_sinks[target_sink_count]);

        target_sink_count++;
    }

    if (target_sink_count > 0) {
        // Compose to single sink
        core_log_sink_list_open(
                target_sinks,
                target_sink_count,
                &list_sink);

        // Async sink only makes sense if at least one other
        // sink is enabled
        if (config->sink_async.enable) {
            core_log_sink_async_open(
                config->msg_buffer_size_bytes,
                config->sink_async.queue_length,
                config->sink_async.overflow_policy,
                &list_sink,
                root_sink);
        } else {
            // "Sync" with list of sinks
            memcpy(root_sink, &list_sink, sizeof(core_log_sink_t));
        }

        return true;
    } else {
        memset(root_sink, 0, sizeof(core_log_sink_t));
        return false;
    }
}

static void _logger_with_sinks_init(const logger_config_t *config)
{
    core_log_sink_t sink;
    bool has_sinks;

    log_assert(config);

    has_sinks = _logger_sinks_create(config, &sink);

    if (has_sinks) {
        core_log_bt_init(config->msg_buffer_size_bytes, &sink);
        core_log_bt_level_set(config->level);
    } else {
        // Consider this equivalent to disabling logging entirely
        _logger_null_sink_init();
    }
}

void logger_init(const logger_config_t *config)
{
    log_assert(config);

    if (!config->enable) {
        _logger_null_sink_init();
    } else {
        _logger_with_sinks_init(config);
    }

    core_log_bt_core_api_set();
}

void logger_fini()
{
    core_log_bt_fini();
}
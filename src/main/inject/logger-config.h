#ifndef INJECT_LOGGER_CONFIG_H
#define INJECT_LOGGER_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#include "core/log-bt.h"
#include "core/log-sink-async.h"
#include "core/property-node.h"

typedef struct logger_config {
    bool enable;
    enum core_log_bt_log_level level;
    uint32_t msg_buffer_size_bytes;

    struct logger_sink_async_config {
        bool enable;
        uint8_t queue_length;
        enum core_log_sink_async_overflow_policy overflow_policy;
    } sink_async;

    struct logger_sink_console_config {
        bool enable;
        bool color;
    } sink_console;

    struct logger_sink_file_config {
        bool enable;
        char path[MAX_PATH];
        bool append;
        bool rotate;
        uint8_t max_rotations;
    } sink_file;
} logger_config_t;

void logger_config_init(logger_config_t *config);

void logger_config_load(
    const core_property_node_t *node, logger_config_t *config);

#endif
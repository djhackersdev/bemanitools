#ifndef BT_SDK_HOOK_INJECT_LOGGER_CONFIG_H
#define BT_SDK_HOOK_INJECT_LOGGER_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#include "core/log-bt.h"
#include "core/log-sink-async.h"
#include "core/property-node.h"

typedef struct bt_hook_logger_config {
    bool enable;
    enum core_log_bt_log_level level;
} bt_hook_logger_config_t;

void bt_hook_logger_config_init(bt_hook_logger_config_t *config);

void bt_hook_logger_config_load(
    const core_property_node_t *node, bt_hook_logger_config_t *config);

#endif
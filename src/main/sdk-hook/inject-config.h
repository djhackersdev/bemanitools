#ifndef BT_SDK_HOOK_INJECT_CONFIG_H
#define BT_SDK_HOOK_INJECT_CONFIG_H

#include <windows.h>

#include "sdk-hook/hooks-config.h"
#include "sdk-hook/logger-config.h"

typedef struct bt_hook_inject_config {
    uint32_t version;

    bt_hook_hooks_config_t hooks;
    bt_hook_logger_config_t logger;
} bt_hook_inject_config_t;

void bt_hook_inject_config_init(struct bt_hook_inject_config *config);

void bt_hook_inject_config_file_load(const char *path, bt_hook_inject_config_t *config);

void bt_hook_inject_config_fini(bt_hook_inject_config_t *config);

#endif
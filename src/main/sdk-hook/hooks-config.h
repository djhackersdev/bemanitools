#ifndef BT_SDK_HOOK_INJECT_HOOKS_CONFIG_H
#define BT_SDK_HOOK_INJECT_HOOKS_CONFIG_H

#include <windows.h>

#include <stdbool.h>

#include "core/property.h"
#include "core/property-node.h"

#define BT_HOOK_HOOKS_CONFIG_MAX_HOOKS 16

typedef struct bt_hook_hooks_config {
    struct bt_hook_hooks_hook_config {
        bool enable;
        char path[MAX_PATH];
        core_property_t *config;
    } hooks[BT_HOOK_HOOKS_CONFIG_MAX_HOOKS];
} bt_hook_hooks_config_t;

void bt_hook_hooks_config_init(bt_hook_hooks_config_t *config);

void bt_hook_hooks_config_load(
    const core_property_node_t *node, bt_hook_hooks_config_t *config);

void bt_hook_hooks_config_fini(bt_hook_hooks_config_t *config);

#endif
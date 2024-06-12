#ifndef BT_HOOK_H
#define BT_HOOK_H

#include <stdbool.h>
#include <stdint.h>

#include "api/hook.h"

typedef struct bt_hook bt_hook_t;

void bt_hook_init(const bt_hook_api_t *api, const char *name, bt_hook_t **hook);
void bt_hook_fini(bt_hook_t **hook);

bool bt_hook_pre_avs_init(
    const bt_hook_t *hook, const bt_core_config_t *config);
void bt_hook_iat_dll_name_get(const bt_hook_t *hook, char *buffer, size_t size);
bool bt_hook_main_init(
    const bt_hook_t *hook, HMODULE game_module, const bt_core_config_t *config);
void bt_hook_main_fini(const bt_hook_t *hook);

#endif
#ifndef BT_HOOKS_H
#define BT_HOOKS_H

#include <windows.h>

#include "bt/hook.h"

#include "imports/avs.h"

void bt_hooks_init();

void bt_hooks_hook_load(const char *path, struct property_node *node);

void bt_hooks_core_thread_impl_set_invoke();

void bt_hooks_core_log_impl_set_invoke();

void bt_hooks_before_avs_init_invoke();

void bt_hooks_iat_apply(HMODULE game_module);

void bt_hooks_main_init_invoke(HMODULE game_module);

void bt_hooks_main_fini_invoke();

void bt_hooks_fini();

#endif
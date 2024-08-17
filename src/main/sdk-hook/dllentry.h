#ifndef BT_SDK_HOOK_DLLENTRY_H
#define BT_SDK_HOOK_DLLENTRY_H

#include <windows.h>

#include "main/module/core.h"
#include "main/module/hook.h"

void bt_hook_dllentry_init(
    HMODULE module,
    const char *name,
    bt_module_core_config_api_set_t hook_config_api_set,
    bt_module_core_log_api_set_t hook_log_api_set,
    bt_module_core_thread_api_set_t hook_thread_api_set,
    bt_module_hook_api_get_t hook_api_get);

void bt_hook_dllentry_fini();

void bt_hook_dllentry_main_init();

void bt_hook_dllentry_main_fini();

#endif
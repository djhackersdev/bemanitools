#ifndef PROCMON_BT_HOOK_H
#define PROCMON_BT_HOOK_H

#include "btapi/hook/core.h"
#include "btapi/hook/main.h"

void bt_hook_core_thread_impl_set(const bt_core_thread_impl_t *impl);
void bt_hook_core_log_impl_set(const bt_core_log_impl_t *impl);
void bt_hook_core_config_impl_set(const bt_core_config_impl_t *impl);
bool bt_hook_main_init(HMODULE game_module, const bt_core_config_t *config);
void bt_hook_main_fini();

#endif
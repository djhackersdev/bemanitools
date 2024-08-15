#ifndef IIDXHOOK1_H
#define IIDXHOOK1_H

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

void bt_module_core_config_api_set(const bt_core_config_api_t *api);
void bt_module_core_log_api_set(const bt_core_log_api_t *api);
void bt_module_core_thread_api_set(const bt_core_thread_api_t *api);
void bt_module_hook_api_get(bt_hook_api_t *api);

#endif
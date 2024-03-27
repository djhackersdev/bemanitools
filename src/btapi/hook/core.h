#ifndef BT_HOOK_CORE_H
#define BT_HOOK_CORE_H

#include "core/config.h"
#include "core/log.h"
#include "core/thread.h"

typedef void (*bt_hook_core_thread_impl_set_t)(const bt_core_thread_impl_t *impl);
typedef void (*bt_hook_core_log_impl_set_t)(const bt_core_log_impl_t *impl);
typedef void (*bt_hook_core_config_impl_set_t)(const bt_core_config_impl_t *impl);

void bt_hook_core_thread_impl_set(const bt_core_thread_impl_t *impl);
void bt_hook_core_log_impl_set(const bt_core_log_impl_t *impl);
void bt_hook_core_config_impl_set(const bt_core_config_impl_t *impl);

#endif
#ifndef BT_HOOK_H
#define BT_HOOK_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "btapi/hook-before-avs.h"
#include "btapi/hook-core.h"
#include "btapi/hook-iat.h"
#include "btapi/hook-main.h"
#include "btapi/log.h"
#include "btapi/thread.h"

#include "imports/avs.h"

struct bt_hook {
    char path[MAX_PATH];
    HMODULE module;
    btapi_hook_core_thread_impl_set_t core_thread_impl_set;
    btapi_hook_core_log_impl_set_t core_log_impl_set;
    btapi_hook_before_avs_init_t before_avs_init;
    btapi_hook_iat_dll_name_get_t iat_dll_name_get;
    btapi_hook_main_init_t main_init;
    btapi_hook_main_fini_t main_fini;
};

void bt_hook_load(const char *path, struct bt_hook *hook);

const char *bt_hook_path_get(const struct bt_hook *hook);
void bt_hook_core_thread_impl_set_invoke(
    const struct bt_hook *hook,
    btapi_thread_create_t create,
    btapi_thread_join_t join,
    btapi_thread_destroy_t destroy);
void bt_hook_core_log_impl_set_invoke(
    const struct bt_hook *hook,
    btapi_log_formatter_t misc,
    btapi_log_formatter_t info,
    btapi_log_formatter_t warning,
    btapi_log_formatter_t fatal);
bool bt_hook_before_avs_init_invoke(const struct bt_hook *hook, struct property_node *config);
void bt_hook_iat_apply(const struct bt_hook *hook, HMODULE game_module);
bool bt_hook_main_init_invoke(const struct bt_hook *hook, HMODULE game_module, struct property_node *config);
void bt_hook_main_fini_invoke(const struct bt_hook *hook);

void bt_hook_free(struct bt_hook *hook);

#endif
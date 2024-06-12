#ifndef EXCEPTIONTRACE_HOOK_H
#define EXCEPTIONTRACE_HOOK_H

#include "btapi/hook-core.h"
#include "btapi/hook-main.h"

void btapi_hook_core_thread_impl_set(
    btapi_thread_create_t create,
    btapi_thread_join_t join,
    btapi_thread_destroy_t destroy);
void btapi_hook_core_log_impl_set(
    btapi_log_formatter_t misc,
    btapi_log_formatter_t info,
    btapi_log_formatter_t warning,
    btapi_log_formatter_t fatal);
bool btapi_hook_main_init(
    HMODULE game_module, struct property_node *property_node_config);
void btapi_hook_main_fini();

#endif
#define LOG_MODULE "exceptiontrance-hook"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "core/log.h"
#include "core/thread.h"

#include "exceptiontrace/exceptiontrace.h"

void btapi_hook_core_thread_impl_set(
    btapi_thread_create_t create,
    btapi_thread_join_t join,
    btapi_thread_destroy_t destroy)
{
    core_thread_impl_set(create, join, destroy);
}

void btapi_hook_core_log_impl_set(
    btapi_log_formatter_t misc,
    btapi_log_formatter_t info,
    btapi_log_formatter_t warning,
    btapi_log_formatter_t fatal)
{
    core_log_impl_set(misc, info, warning, fatal);
}

bool btapi_hook_main_init(
    HMODULE game_module, struct property_node *property_node_config)
{
    exceptiontrace_init(core_log_fatal_impl_get());

    return true;
}

void btapi_hook_main_fini()
{
    // noop
}
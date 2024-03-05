#define LOG_MODULE "procmon-hook"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "btapi/hook-core.h"
#include "btapi/hook-main.h"

#include "core/log.h"
#include "core/thread.h"

#include "procmon/config.h"
#include "procmon/procmon.h"

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
    struct procmon_config config;

    log_assert(game_module);
    log_assert(property_node_config);

    procmon_config_init(&config);
    procmon_config_load(property_node_config, &config);

    procmon_init(&config);

    return true;
}

void btapi_hook_main_fini()
{
    procmon_fini();
}
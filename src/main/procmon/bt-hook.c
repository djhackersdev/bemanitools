#define LOG_MODULE "procmon-hook"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "btapi/hook-core.h"
#include "btapi/hook-main.h"

#include "btsdk/core/config.h"
#include "btsdk/core/log.h"

#include "procmon/config.h"
#include "procmon/procmon.h"

void bt_hook_core_log_impl_set(const bt_core_log_impl_t *impl)
{
    bt_core_log_impl_set(impl);
}

void bt_hook_core_config_impl_set(const bt_core_config_impl_t *impl)
{
    bt_core_config_impl_set(impl);
}

bool bt_hook_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    procmon_config_t procmon_config;

    procmon_config_load(config, &procmon_config);

    procmon_init(&procmon_config);

    return true;
}

void bt_hook_main_fini()
{
    procmon_fini();
}
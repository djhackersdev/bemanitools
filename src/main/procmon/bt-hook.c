#define LOG_MODULE "procmon-hook"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "iface-core/config.h"
#include "iface-core/log.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/hook.h"

#include "procmon/config.h"
#include "procmon/procmon.h"

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

void bt_module_core_config_api_set(const bt_core_config_api_t *api)
{
    bt_core_config_api_set(api);
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_hook_api_get(bt_hook_api_t *api)
{
    api->version = 1;

    api->v1.main_init = bt_hook_main_init;
    api->v1.main_fini = bt_hook_main_fini;
}
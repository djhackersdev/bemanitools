#define LOG_MODULE "hook-dllentry"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <shlwapi.h>
// clang-format on

#include <stdbool.h>

#include "core/boot.h"
#include "core/config-property-node.h"
#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/property-ext.h"
#include "core/property-node.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface/hook.h"

#include "iidxhook1/iidxhook1.h"

#include "main/module/core.h"
#include "main/module/hook.h"

#include "sdk-hook/inject-config.h"

#include "util/str.h"

static HMODULE _bt_hook_dllentry_hook_module;
static bt_hook_t *_bt_hook_dllentry_hook;
static bt_hook_inject_config_t _bt_hook_dllentry_inject_config;

static void _bt_hook_dllentry_current_module_name_get(char *name, size_t len)
{
    char path[MAX_PATH];
    const char *filename;

    log_assert(name);
    log_assert(len > 0);

    if (GetModuleFileName(_bt_hook_dllentry_hook_module, path, MAX_PATH) != 0) {
        filename = PathFindFileName(path);

        str_cpy(name, len, filename);
    } else {
        log_fatal("GetModuleFileName to get current module name failed");
    }
}

static const core_property_t *_bt_hook_dllentry_hook_property_config_get()
{
    char current_module_name[MAX_PATH];
    const struct bt_hook_hooks_hook_config *configs;
    const char *filename;
    int i;

    _bt_hook_dllentry_current_module_name_get(current_module_name, sizeof(current_module_name));

    configs = _bt_hook_dllentry_inject_config.hooks.hooks;

    for (i = 0; i < BT_HOOK_HOOKS_CONFIG_MAX_HOOKS; i++) {
        filename = PathFindFileName(configs[i].path);

        if (str_eq(current_module_name, filename)) {
            log_misc("Property hook config for %s", configs[i].path);
            core_property_ext_log(configs[i].config, log_misc_func);

            return configs[i].config;
        }
    }

    log_fatal("Could not find hook configuration for module %s in inject configuration", current_module_name);
}

static void _bt_hook_dllentry_config_load()
{
    char inject_config_path[MAX_PATH];

    // Duct-tape to allow access to config path and load config in hook dlls
    GetEnvironmentVariable("INJECT_CONFIG_PATH", inject_config_path, sizeof(inject_config_path));

    // With DllMain as the only callable entry point from inject, 
    // the only way to get the configuration for the hook is through
    // injects configuration file + command line arguments
    // Thus, the whole loading process (that is also done in inject)
    // is replicated here (inject config etc. are also copy-pasted from
    // the inject module)
    bt_hook_inject_config_init(&_bt_hook_dllentry_inject_config);
    bt_hook_inject_config_file_load(inject_config_path, &_bt_hook_dllentry_inject_config);
}

static void _bt_hook_dllentry_logger_reinit()
{
    // Apply same logger configurations as inject
    if (!_bt_hook_dllentry_inject_config.logger.enable) {
        core_log_bt_fini();

        core_log_bt_ext_init_with_null();
    } else {
        // Logger already initialized previously, just switch log level
        core_log_bt_level_set(_bt_hook_dllentry_inject_config.logger.level);
    }
}

void bt_hook_dllentry_init(
    HMODULE module, 
    const char* name,
    bt_module_core_config_api_set_t hook_config_api_set,
    bt_module_core_log_api_set_t hook_log_api_set,
    bt_module_core_thread_api_set_t hook_thread_api_set,
    bt_module_hook_api_get_t hook_api_get)
{
    bt_core_config_api_t config_api;
    bt_core_log_api_t log_api;
    bt_core_thread_api_t thread_api;
    bt_hook_api_t hook_api;

    core_boot_dll(name);

    // Debug logging is captured by debugger in inject and actually
    // sunk to outputs there, e.g. terminal/file
    core_log_bt_ext_init_with_debug();
    core_log_bt_core_api_set();
    // TODO change log level according to how inject is configured, read from config + command line args
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

    _bt_hook_dllentry_config_load();

    _bt_hook_dllentry_logger_reinit();

    // Assert after log init to get visible error output if these fail
    log_assert(module);
    log_assert(hook_config_api_set);
    log_assert(hook_log_api_set);
    log_assert(hook_thread_api_set);
    log_assert(hook_api_get);

    _bt_hook_dllentry_hook_module = module;

    bt_core_config_api_get(&config_api);
    bt_core_log_api_get(&log_api);
    bt_core_thread_api_get(&thread_api);

    hook_config_api_set(&config_api);
    hook_log_api_set(&log_api);
    hook_thread_api_set(&thread_api);

    hook_api_get(&hook_api);

    bt_hook_init(&hook_api, name, &_bt_hook_dllentry_hook);

    log_misc("<<< bt_hook_dllentry_boot_init");
}

void bt_hook_dllentry_fini()
{
    log_misc(">>> bt_hook_dllentry_fini");

    bt_hook_inject_config_fini(&_bt_hook_dllentry_inject_config);

    bt_hook_fini(&_bt_hook_dllentry_hook);

    core_log_bt_fini();

    bt_core_config_api_clear();
    bt_core_log_api_clear();
    bt_core_thread_api_clear();
}

void bt_hook_dllentry_main_init()
{
    bool result;
    const core_property_t *property;
    core_property_node_t root_node;
    bt_core_config_t *config;
    core_property_result_t result_prop;
    HMODULE game_module;

    log_misc(">>> bt_hook_dllentry_main_init");

    property = _bt_hook_dllentry_hook_property_config_get();

    result_prop = core_property_root_node_get(property, &root_node);
    core_property_fatal_on_error(result_prop);

    core_config_property_node_init(&root_node, &config);

    // Because this is loaded into the game's process space, this yields the executable module
    game_module = GetModuleHandle(NULL);

    result = bt_hook_main_init(_bt_hook_dllentry_hook, game_module, config);

    core_config_property_node_free(&config);

    if (!result) {
        log_fatal("Calling hook main init failed");
    }

    log_misc("<<< bt_hook_dllentry_main_init");
}

void bt_hook_dllentry_main_fini()
{
    log_misc(">>> bt_hook_dllentry_boot_fini");

    bt_hook_main_fini(_bt_hook_dllentry_hook);

    bt_hook_fini(&_bt_hook_dllentry_hook);

    log_misc("<<< bt_hook_dllentry_boot_fini");
}

#define LOG_MODULE "module-hook"

#include <windows.h>

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "main/module/core.h"
#include "main/module/hook.h"
#include "main/module/module.h"

#include "util/mem.h"
#include "util/str.h"

typedef void (*module_hook_api_get_t)(bt_hook_api_t *api);

struct module_hook {
    module_t *module;

    bt_module_core_config_api_set_t core_config_api_set;
    bt_module_core_log_api_set_t core_log_api_set;
    bt_module_core_thread_api_set_t core_thread_api_set;

    module_hook_api_get_t api_get;
};

static void _module_hook_resolve(module_hook_t *module)
{
    module->core_config_api_set =
        (bt_module_core_config_api_set_t) module_func_optional_resolve(
            module->module, "bt_module_core_config_api_set");
    module->core_log_api_set =
        (bt_module_core_log_api_set_t) module_func_optional_resolve(
            module->module, "bt_module_core_log_api_set");
    module->core_thread_api_set =
        (bt_module_core_thread_api_set_t) module_func_optional_resolve(
            module->module, "bt_module_core_thread_api_set");

    module->api_get = (module_hook_api_get_t) module_func_optional_resolve(
        module->module, "bt_module_hook_api_get");
}

void module_hook_load(const char *path, module_hook_t **module)
{
    log_assert(path);
    log_assert(module);

    *module = xmalloc(sizeof(module_hook_t));
    memset(*module, 0, sizeof(module_hook_t));

    module_load(path, &(*module)->module);

    _module_hook_resolve(*module);
}

const char *module_hook_path_get(const module_hook_t *module)
{
    log_assert(module);

    return module_path_get(module->module);
}

void module_hook_free(module_hook_t **module)
{
    log_assert(module);

    module_free(&(*module)->module);
    memset(*module, 0, sizeof(module_hook_t));
}

void module_hook_core_config_api_set(
    const module_hook_t *module, const bt_core_config_api_t *api)
{
    log_assert(module);
    log_assert(api);

    if (module->core_config_api_set) {
        module_func_pre_invoke_log(
            module->module, "bt_module_core_config_api_set");

        module->core_config_api_set(api);

        module_func_post_invoke_log(
            module->module, "bt_module_core_config_api_set");
    }
}

void module_hook_core_log_api_set(
    const module_hook_t *module, const bt_core_log_api_t *api)
{
    log_assert(module);
    log_assert(api);

    if (module->core_log_api_set) {
        module_func_pre_invoke_log(
            module->module, "bt_module_core_log_api_set");

        module->core_log_api_set(api);

        module_func_post_invoke_log(
            module->module, "bt_module_core_log_api_set");
    }
}

void module_hook_core_thread_api_set(
    const module_hook_t *module, const bt_core_thread_api_t *api)
{
    log_assert(module);
    log_assert(api);

    if (module->core_thread_api_set) {
        module_func_pre_invoke_log(
            module->module, "bt_module_core_thread_api_set");

        module->core_thread_api_set(api);

        module_func_post_invoke_log(
            module->module, "bt_module_core_thread_api_set");
    }
}

void module_hook_api_get(const module_hook_t *module, bt_hook_api_t *api)
{
    log_assert(module);
    log_assert(api);

    memset(api, 0, sizeof(bt_hook_api_t));

    if (module->api_get) {
        module_func_pre_invoke_log(module->module, "bt_hook_api_get");

        module->api_get(api);

        module_func_post_invoke_log(module->module, "bt_hook_api_get");
    } else {
        // Consider this still v1 as all functions are optional
        // This allows for creating rather simple and even bemanitools
        // independent hook libraries that only use DllMain
        api->version = 1;
    }
}

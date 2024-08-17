#define LOG_MODULE "module-input"

#include "api/input.h"
#include "api/core/log.h"
#include "api/core/thread.h"

#include "iface-core/log.h"

#include "main/module/core.h"
#include "main/module/input.h"
#include "main/module/module.h"

#include "util/mem.h"

typedef void (*module_input_api_get_t)(bt_input_api_t *api);

struct module_input {
    module_t *module;

    bt_module_core_log_api_set_t core_log_api_set;
    bt_module_core_thread_api_set_t core_thread_api_set;

    module_input_api_get_t api_get;
};

static void _module_input_resolve(module_input_t *module)
{
    module->core_log_api_set =
        (bt_module_core_log_api_set_t) module_func_optional_resolve(
            module->module, "bt_module_core_log_api_set");
    module->core_thread_api_set =
        (bt_module_core_thread_api_set_t) module_func_optional_resolve(
            module->module, "bt_module_core_thread_api_set");

    module->api_get = (module_input_api_get_t) module_func_required_resolve(
        module->module, "bt_module_input_api_get");
}

void module_input_load(const char *path, module_input_t **module)
{
    log_assert(path);
    log_assert(module);

    *module = xmalloc(sizeof(module_input_t));
    memset(*module, 0, sizeof(module_input_t));

    module_load(path, &(*module)->module);

    _module_input_resolve(*module);
}

void module_input_free(module_input_t **module)
{
    log_assert(module);

    module_free(&(*module)->module);
    memset(*module, 0, sizeof(module_input_t));
}

void module_input_core_log_api_set(
    const module_input_t *module, const bt_core_log_api_t *api)
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

void module_input_core_thread_api_set(
    const module_input_t *module, const bt_core_thread_api_t *api)
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

void module_input_api_get(const module_input_t *module, bt_input_api_t *api)
{
    log_assert(module);
    log_assert(api);

    module_func_pre_invoke_log(module->module, "bt_module_input_api_get");

    module->api_get(api);

    module_func_post_invoke_log(module->module, "bt_module_input_api_get");
}
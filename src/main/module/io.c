
#define LOG_MODULE "module-io"

#include "api/core/config.h"
#include "api/core/thread.h"

#include "iface-core/log.h"

#include "main/module/core.h"
#include "main/module/io.h"
#include "main/module/module.h"

#include "util/mem.h"
#include "util/str.h"

typedef void (*module_io_api_get_t)(void *api);

struct module_io {
    module_t *module;

    bt_module_core_config_api_set_t core_config_api_set;
    bt_module_core_log_api_set_t core_log_api_set;
    bt_module_core_thread_api_set_t core_thread_api_set;

    module_io_api_get_t api_get;
    // Keep for debug logging
    char api_get_func_name[4096];
};

static void
_module_io_resolve(module_io_t *module, const char *api_get_func_name)
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

    module->api_get = (module_io_api_get_t) module_func_required_resolve(
        module->module, api_get_func_name);
    str_cpy(
        module->api_get_func_name,
        sizeof(module->api_get_func_name),
        api_get_func_name);
}

void module_io_load(
    const char *path, const char *api_get_func_name, module_io_t **module)
{
    log_assert(path);
    log_assert(api_get_func_name);
    log_assert(module);

    *module = xmalloc(sizeof(module_io_t));
    memset(*module, 0, sizeof(module_io_t));

    module_load(path, &(*module)->module);

    _module_io_resolve(*module, api_get_func_name);
}

void module_io_free(module_io_t **module)
{
    log_assert(module);

    module_free(&(*module)->module);
    memset(*module, 0, sizeof(module_io_t));
}

void module_io_core_config_api_set(
    const module_io_t *module, const bt_core_config_api_t *api)
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

void module_io_core_log_api_set(
    const module_io_t *module, const bt_core_log_api_t *api)
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

void module_io_core_thread_api_set(
    const module_io_t *module, const bt_core_thread_api_t *api)
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

void module_io_api_get(const module_io_t *module, void *api)
{
    log_assert(module);
    log_assert(api);

    module_func_pre_invoke_log(module->module, module->api_get_func_name);

    module->api_get(api);

    module_func_post_invoke_log(module->module, module->api_get_func_name);
}
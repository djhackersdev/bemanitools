#define LOG_MODULE "module-acio_mgr"

#include "api/acio/mgr.h"
#include "api/core/log.h"

#include "iface-core/log.h"

#include "main/module/acio-mgr.h"
#include "main/module/core.h"
#include "main/module/module.h"

#include "util/mem.h"

typedef void (*module_acio_mgr_api_get_t)(bt_acio_mgr_api_t *api);

struct module_acio_mgr {
    module_t *module;

    bt_module_core_log_api_set_t core_log_api_set;

    module_acio_mgr_api_get_t api_get;
};

static void _module_acio_mgr_resolve(module_acio_mgr_t *module)
{
    module->core_log_api_set =
        (bt_module_core_log_api_set_t) module_func_optional_resolve(
            module->module, "bt_module_core_log_api_set");

    module->api_get = (module_acio_mgr_api_get_t) module_func_required_resolve(
        module->module, "bt_acio_mgr_api_get");
}

void module_acio_mgr_load(const char *path, module_acio_mgr_t **module)
{
    log_assert(path);
    log_assert(module);

    *module = xmalloc(sizeof(module_acio_mgr_t));
    memset(*module, 0, sizeof(module_acio_mgr_t));

    module_load(path, &(*module)->module);

    _module_acio_mgr_resolve(*module);
}

void module_acio_mgr_free(module_acio_mgr_t **module)
{
    log_assert(module);

    module_free(&(*module)->module);
    memset(*module, 0, sizeof(module_acio_mgr_t));
}

void module_acio_mgr_core_log_api_set(
    const module_acio_mgr_t *module, const bt_core_log_api_t *api)
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

void module_acio_mgr_api_get(
    const module_acio_mgr_t *module, bt_acio_mgr_api_t *api)
{
    log_assert(module);
    log_assert(api);

    module_func_pre_invoke_log(module->module, "bt_acio_mgr_api_get");

    module->api_get(api);

    module_func_post_invoke_log(module->module, "bt_acio_mgr_api_get");
}
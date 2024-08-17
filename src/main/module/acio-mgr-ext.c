#include "iface-core/log.h"

#include "module/acio-mgr.h"

void module_acio_mgr_ext_load_and_init(
    const char *path, module_acio_mgr_t **module)
{
    bt_core_log_api_t core_log_api;

    module_acio_mgr_load(path, module);

    bt_core_log_api_get(&core_log_api);

    module_acio_mgr_core_log_api_set(*module, &core_log_api);
}
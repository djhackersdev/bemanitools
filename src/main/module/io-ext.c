#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "module/io.h"

void module_io_ext_load_and_init(
    const char *path, const char *api_get_func_name, module_io_t **module)
{
    // TODO currently not supported
    // bt_core_config_api_t core_config_api;
    bt_core_log_api_t core_log_api;
    bt_core_thread_api_t core_thread_api;

    module_io_load(path, api_get_func_name, module);

    // TODO currently not supported
    // bt_core_config_api_get(&core_config_api);
    bt_core_log_api_get(&core_log_api);
    bt_core_thread_api_get(&core_thread_api);

    // TODO currently not supported
    // module_io_core_config_api_set(*module, &core_config_api);
    module_io_core_log_api_set(*module, &core_log_api);
    module_io_core_thread_api_set(*module, &core_thread_api);
}
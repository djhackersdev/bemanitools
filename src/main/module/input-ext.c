#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "module/input.h"

void module_input_ext_load_and_init(const char *path, module_input_t **module)
{
    bt_core_log_api_t core_log_api;
    bt_core_thread_api_t core_thread_api;

    module_input_load(path, module);

    bt_core_log_api_get(&core_log_api);
    bt_core_thread_api_get(&core_thread_api);

    module_input_core_log_api_set(*module, &core_log_api);
    module_input_core_thread_api_set(*module, &core_thread_api);
}
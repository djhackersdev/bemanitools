#define LOG_MODULE "procmon-dllmain"

#include <windows.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/thread-crt-ext.h"
#include "launcher/property-util.h"

#include "procmon/config.h"
#include "procmon/procmon.h"

#include "util/fs.h"

// TODO move this to a separate "standalone dllmain bootstrap" module in sdk

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    struct property *property;
    struct property_node *root_node;
    struct procmon_config config;

    // TODO provide compatibility mode here? ctx set to...something?
    // to figure out if this is loaded/run by any bemanitools loader
    // or not.

    if (reason == DLL_PROCESS_ATTACH) {
        core_thread_crt_ext_impl_set();
        core_log_bt_ext_init_with_stderr();

        // TODO make this somehow configurable?
        // have additional stuff on procmon configuration xml
        // <dllmain>
        //   <log>
        //      have the same log stuff here as with launcher like file, level, rotation etc
        //   </log>
        //   <procmon>
        //    ....
        //   </procmon>
        // </dllmain>
        core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_INFO);

        log_info("DLL_PROCESS_ATTACH");

        if (path_exists("procmon-config.xml")) {
            property = property_util_load("procmon-config.xml");

            root_node = property_search(NULL, "hook/procmon");

            if (!root_node) {
                // TODO error handling
            }

            procmon_config_init(&config);
            procmon_config_load(root_node, &config);

            procmon_init(&config);
        } else {
            log_warning("No configuration file found, defaulting");

            procmon_config_init(&config);

            procmon_init(&config);
        }
        
    } else if (reason == DLL_PROCESS_DETACH) {
        log_info("DLL_PROCESS_DETACH");

        procmon_fini();
        core_log_bt_fini();
    }

    return TRUE;
}

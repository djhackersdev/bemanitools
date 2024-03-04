#include <windows.h>

#include "core/log-bt-ext.h"
#include "core/log.h"
#include "core/thread-crt-ext.h"
#include "core/thread.h"

#include "procmon/procmon.h"

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        core_thread_crt_ext_impl_set();
        core_log_bt_ext_impl_set();

        core_log_bt_ext_init_with_stdout();


    //     core_log_bt_ext_init_with_stdout_and_file(
    // const char *path, bool append, bool rotate, uint8_t max_rotations);


        procmon_init();

        procmon_file_mon_enable();
        procmon_module_mon_enable();
        procmon_thread_mon_enable();
    } else if (reason == DLL_PROCESS_DETACH) {
        procmon_fini();
    }

    return TRUE;
}

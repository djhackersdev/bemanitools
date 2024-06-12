#include <windows.h>

#include "acio-mgr/internal.h"

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        bt_acio_mgr_init();
    }

    if (reason == DLL_PROCESS_DETACH) {
        bt_acio_mgr_fini();
    }

    return TRUE;
}

#include <windows.h>

#include "manager-init.h"

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        _aciomgr_init();
    }

    if (reason == DLL_PROCESS_DETACH) {
        _aciomgr_fini();
    }

    return TRUE;
}

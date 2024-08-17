#include <windows.h>

#include "hook/table.h"

#include "ddrhook1/ddrhook1.h"

#include "main/sdk-hook/dllentry.h"

#include "util/defs.h"

static DWORD STDCALL
_ddrhook1_dllmain_my_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
static DWORD(STDCALL *_ddrhook1_dllmain_real_GetModuleFileNameA)(
    HMODULE hModule, LPSTR lpFilename, DWORD nSize);

static bool _ddrhook1_dllmain_getmodulefilenamea_call_check;

static const struct hook_symbol _ddrhook1_dllmain_hook_syms[] = {
    {
        .name = "GetModuleFileNameA",
        .patch = _ddrhook1_dllmain_my_GetModuleFileNameA,
        .link = (void **) &_ddrhook1_dllmain_real_GetModuleFileNameA,
    },
};

static DWORD STDCALL
_ddrhook1_dllmain_my_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    if (_ddrhook1_dllmain_getmodulefilenamea_call_check) {
        return _ddrhook1_dllmain_real_GetModuleFileNameA(hModule, lpFilename, nSize); 
    }

    _ddrhook1_dllmain_getmodulefilenamea_call_check = true;

    bt_hook_dllentry_main_init();

    return _ddrhook1_dllmain_real_GetModuleFileNameA(hModule, lpFilename, nSize); 
}

// TODO find another call right before main exits to hook cleanup and stuff with
// bt_hook_dllentry_main_fini()

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        bt_hook_dllentry_init(
            mod,
            "ddrhook1",
            bt_module_core_config_api_set,
            bt_module_core_log_api_set,
            bt_module_core_thread_api_set,
            bt_module_hook_api_get);

        _ddrhook1_dllmain_getmodulefilenamea_call_check = false;

        hook_table_apply(
            NULL, "kernel32.dll", _ddrhook1_dllmain_hook_syms, lengthof(_ddrhook1_dllmain_hook_syms));

    } else if (reason == DLL_PROCESS_DETACH) {
        // https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain#remarks
        if (ctx == NULL) {
            hook_table_revert(NULL, "kernel32.dll", _ddrhook1_dllmain_hook_syms, lengthof(_ddrhook1_dllmain_hook_syms));

            // Hacky to have this here, should be close/right after application main exits, see TODO above
            bt_hook_dllentry_main_fini();

            bt_hook_dllentry_fini();
        }
    }

    return TRUE;
}
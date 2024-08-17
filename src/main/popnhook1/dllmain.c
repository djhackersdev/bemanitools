#include <windows.h>

#include "hook/table.h"

#include "popnhook1/popnhook1.h"

#include "main/sdk-hook/dllentry.h"

#include "util/defs.h"

static DWORD STDCALL _popnhook1_dllmain_my_GetStartupInfoA(LPSTARTUPINFOA lpStartupInfo);
static DWORD(STDCALL *_popnhook1_dllmain_real_GetStartupInfoA)(LPSTARTUPINFOA lpStartupInfo);

static bool _popnhook1_dllmain_getstartupinfoa_call_check;

static const struct hook_symbol _popnhook1_dllmain_hook_syms[] = {
    {
        .name = "GetStartupInfoA",
        .patch = _popnhook1_dllmain_my_GetStartupInfoA,
        .link = (void **) &_popnhook1_dllmain_real_GetStartupInfoA,
    },
};

static DWORD STDCALL
_popnhook1_dllmain_my_GetStartupInfoA(LPSTARTUPINFOA lpStartupInfo)
{
    if (_popnhook1_dllmain_getstartupinfoa_call_check) {
        return _popnhook1_dllmain_real_GetStartupInfoA(lpStartupInfo); 
    }

    _popnhook1_dllmain_getstartupinfoa_call_check = true;

    bt_hook_dllentry_main_init();

    return _popnhook1_dllmain_real_GetStartupInfoA(lpStartupInfo);
}

// TODO find another call right before main exits to hook cleanup and stuff with
// bt_hook_dllentry_main_fini()

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        bt_hook_dllentry_init(
            mod,
            "popnhook1",
            bt_module_core_config_api_set,
            bt_module_core_log_api_set,
            bt_module_core_thread_api_set,
            bt_module_hook_api_get);

        _popnhook1_dllmain_getstartupinfoa_call_check = false;

        hook_table_apply(
            NULL, "kernel32.dll", _popnhook1_dllmain_hook_syms, lengthof(_popnhook1_dllmain_hook_syms));

    } else if (reason == DLL_PROCESS_DETACH) {
        // https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain#remarks
        if (ctx == NULL) {
            hook_table_revert(NULL, "kernel32.dll", _popnhook1_dllmain_hook_syms, lengthof(_popnhook1_dllmain_hook_syms));

            // Hacky to have this here, should be close/right after application main exits, see TODO above
            bt_hook_dllentry_main_fini();

            bt_hook_dllentry_fini();
        }
    }

    return TRUE;
}
#include <windows.h>

#include "hook/table.h"

#include "jbhook1/jbhook1.h"

#include "main/sdk-hook/dllentry.h"

#include "util/defs.h"

static HWND CDECL
_jbhook1_dllmain_my_mwindow_create(HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);
static HWND(CDECL *_jbhook1_dllmain_real_mwindow_create)(
    HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);

static const struct hook_symbol _jbhook1_dllmain_hook_syms[] = {
    {
        .name = "mwindow_create",
        .patch = _jbhook1_dllmain_my_mwindow_create,
        .link = (void **) &_jbhook1_dllmain_real_mwindow_create,
    },
};

/**
 * This seems to be a good entry point to intercept before the game calls
 * anything important (very close to the start of WinMain).
 */
static HWND CDECL _jbhook1_dllmain_my_mwindow_create(
    HINSTANCE hinstance,
    void *callback,
    const char *window_title,
    DWORD window_width,
    DWORD window_height,
    BOOL fullscreen)
{
    bt_hook_dllentry_main_init();

    return _jbhook1_dllmain_real_mwindow_create(
        hinstance,
        callback,
        window_title,
        window_width,
        window_height,
        fullscreen);
}

// TODO find another call right before main exits to hook cleanup and stuff with
// bt_hook_dllentry_main_fini()

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        bt_hook_dllentry_init(
            mod,
            "jbhook1",
            bt_module_core_config_api_set,
            bt_module_core_log_api_set,
            bt_module_core_thread_api_set,
            bt_module_hook_api_get);

        hook_table_apply(
            NULL, "mwindow.dll", _jbhook1_dllmain_hook_syms, lengthof(_jbhook1_dllmain_hook_syms));

    } else if (reason == DLL_PROCESS_DETACH) {
        // https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain#remarks
        if (ctx == NULL) {
            hook_table_revert(NULL, "mwindow.dll", _jbhook1_dllmain_hook_syms, lengthof(_jbhook1_dllmain_hook_syms));

            // Hacky to have this here, should be close/right after application main exits, see TODO above
            bt_hook_dllentry_main_fini();

            bt_hook_dllentry_fini();
        }
    }

    return TRUE;
}
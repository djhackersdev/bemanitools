#include <windows.h>

#include <stdbool.h>

#include "core/boot.h"
#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/thread-crt.h"

#include "hook/table.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface/hook.h"

#include "iidxhook1/iidxhook1.h"

static HANDLE STDCALL _iidxhook1_my_OpenProcess(DWORD, BOOL, DWORD);
static HANDLE(STDCALL *_iidxhook1_real_OpenProcess)(DWORD, BOOL, DWORD);

static bool _iidxhook1_hook_main_call_check;
static bt_hook_t *_iidxhook1_hook;

static const struct hook_symbol _iidxhook1_hook_syms[] = {
    {
        .name = "OpenProcess",
        .patch = _iidxhook1_my_OpenProcess,
        .link = (void **) &_iidxhook1_real_OpenProcess,
    },
};

static HANDLE STDCALL
_iidxhook1_my_OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
    bool result;

    if (_iidxhook1_hook_main_call_check) {
        return _iidxhook1_real_OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId); 
    }

    _iidxhook1_hook_main_call_check = true;

    // TODO pass actual hmodule and config, not used right now, so just passes asserts
    result = bt_hook_main_init(_iidxhook1_hook, (HMODULE) 1, (const bt_core_config_t *) 1);

    if (!result) {
        log_fatal("Invoking bt_hook_main_init failed");
    }

    return _iidxhook1_real_OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
}

static void _iidxhook1_dllmain_bt_hook_env_init()
{
    bt_hook_api_t api;

    core_boot_dll("iidxhook1");

    core_thread_crt_core_api_set();

    // Debug logging is captured by debugger in inject and actually
    // sunk to outputs there, e.g. terminal/file
    core_log_bt_ext_init_with_debug();

    core_log_bt_core_api_set();
    
    // TODO change log level support
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

    
    
    bt_module_hook_api_get(&api);

    bt_hook_init(&api, "iidxhook1", &_iidxhook1_hook);

    _iidxhook1_hook_main_call_check = false;

    hook_table_apply(
        NULL, "kernel32.dll", _iidxhook1_hook_syms, lengthof(_iidxhook1_hook_syms));

    log_misc("<<< dllmain_bt_hook_env_init");
}

static void _iidxhook1_dllmain_bt_hook_env_fini()
{
    log_misc(">>> dllmain_bt_hook_env_fini");

    bt_hook_main_fini(_iidxhook1_hook);

    hook_table_revert(NULL, "kernel32.dll", _iidxhook1_hook_syms, lengthof(_iidxhook1_hook_syms));

    bt_hook_fini(&_iidxhook1_hook);

    core_log_bt_fini();

    bt_core_log_api_clear();
    bt_core_thread_api_clear();
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        _iidxhook1_dllmain_bt_hook_env_init();
    } else if (reason == DLL_PROCESS_DETACH) {
        if (ctx == NULL) {
            // Free library called or DLL load failed
            _iidxhook1_dllmain_bt_hook_env_fini();
        } // else Process terminating
    }

    return TRUE;
}
#include <windows.h>

#include <stdbool.h>

#include "hook/iohook.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "unicorntail/p3io.h"
#include "unicorntail/usbmem.h"

#include "util/defs.h"

static bool
_unicorntail_main_init(HMODULE game_module, const bt_core_config_t *config_)
{
    log_info("--- Begin unicorntail dll_entry_init ---");

    iohook_push_handler(p3io_filter_dispatch_irp);
    iohook_push_handler(usbmem_dispatch_irp);

    p3io_filter_init();
    usbmem_init();

    log_info("--- End unicorntail dll_entry_init ---");

    return true;
}

static void _unicorntail_main_fini()
{
    usbmem_fini();
    p3io_filter_fini();
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_hook_api_get(bt_hook_api_t *api)
{
    api->version = 1;

    api->v1.main_init = _unicorntail_main_init;
    api->v1.main_fini = _unicorntail_main_fini;
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    return TRUE;
}

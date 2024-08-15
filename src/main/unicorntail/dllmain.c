#include <windows.h>

#include <stdbool.h>

#include "avs-ext/log.h"
#include "avs-ext/thread.h"

#include "hook/iohook.h"

#include "hooklib/app.h"

#include "iface-core/log.h"

#include "imports/avs.h"

#include "unicorntail/p3io.h"
#include "unicorntail/usbmem.h"

#include "util/defs.h"

static bool my_dll_entry_init(char *sidcode, struct property_node *param);
static bool my_dll_entry_main(void);

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    log_info("--- Begin unicorntail dll_entry_init ---");

    iohook_push_handler(p3io_filter_dispatch_irp);
    iohook_push_handler(usbmem_dispatch_irp);

    p3io_filter_init();
    usbmem_init();

    log_info("--- End unicorntail dll_entry_init ---");

    return app_hook_invoke_init(sidcode, param);
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    usbmem_fini();
    p3io_filter_fini();

    return result;
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    avs_ext_log_core_api_set();
    avs_ext_thread_core_api_set();

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

end:
    return TRUE;
}

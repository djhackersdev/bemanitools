#include <windows.h>

#include <stdbool.h>

#include "ddrhook/p3io.h"
#include "ddrhook/usbmem.h"

#include "hook/iohook.h"

#include "hooklib/app.h"

#include "imports/avs.h"

#include "unicorntail/p3io.h"
#include "unicorntail/usbmem.h"

#include "util/defs.h"
#include "util/log.h"

static bool my_dll_entry_init(char *sidcode, struct property_node *param);
static bool my_dll_entry_main(void);

static const irp_handler_t unicorntail_handlers[] = {
    p3io_filter_dispatch_irp,
    usbmem_dispatch_irp,
};

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    log_info("--- Begin unicorntail dll_entry_init ---");

    iohook_init(unicorntail_handlers, lengthof(unicorntail_handlers));
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

    log_to_external(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

end:
    return TRUE;
}

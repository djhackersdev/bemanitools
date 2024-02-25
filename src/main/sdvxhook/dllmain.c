#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "avs-util/core-interop.h"

#include "bemanitools/eamio.h"
#include "bemanitools/sdvxio.h"

#include "core/log.h"
#include "core/thread.h"

#include "hook/iohook.h"

#include "hooklib/app.h"
#include "hooklib/rs232.h"

#include "sdvxhook/acio.h"
#include "sdvxhook/gfx.h"
#include "sdvxhook/lcd.h"

#include "util/cmdline.h"
#include "util/defs.h"

static bool my_dll_entry_init(char *sidcode, struct property_node *config);
static bool my_dll_entry_main(void);

static bool my_dll_entry_init(char *sidcode, struct property_node *config)
{
    bool ok;

    log_info("--- Begin sdvxhook dll_entry_init ---");

    ac_io_bus_init();

    log_info("Starting up SDVX IO backend");

    core_log_impl_assign(sdvx_io_set_loggers);

    ok = sdvx_io_init(
        core_thread_create_impl_get(),
        core_thread_join_impl_get(),
        core_thread_destroy_impl_get());

    if (!ok) {
        goto sdvx_io_fail;
    }

    log_info("Starting up card reader backend");

    core_log_impl_assign(eam_io_set_loggers);

    ok = eam_io_init(
        core_thread_create_impl_get(),
        core_thread_join_impl_get(),
        core_thread_destroy_impl_get());

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(ac_io_bus_dispatch_irp);
    iohook_push_handler(lcd_dispatch_irp);

    rs232_hook_init();
    lcd_init();

    if (!ok) {
        goto eam_io_fail;
    }

    log_info("---  End  sdvxhook dll_entry_init ---");

    return app_hook_invoke_init(sidcode, config);

eam_io_fail:
    sdvx_io_fini();

sdvx_io_fail:
    ac_io_bus_fini();

    return false;
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    log_info("Shutting down card reader backend");
    eam_io_fini();

    log_info("Shutting down SDVX IO backend");
    sdvx_io_fini();

    ac_io_bus_fini();

    return result;
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    int i;
    int argc;
    char **argv;

    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    // Use AVS APIs
    avs_util_core_interop_thread_avs_impl_set();
    avs_util_core_interop_log_avs_impl_set();

    args_recover(&argc, &argv);

    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            continue;
        }

        switch (argv[i][1]) {
            case 'c':
                gfx_set_confined();

                break;

            case 'w':
                gfx_set_windowed();

                break;
        }
    }

    args_free(argc, argv);

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    gfx_init();

    return TRUE;
}

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "bemanitools/eamio.h"
#include "bemanitools/sdvxio.h"

#include "hook/iohook.h"

#include "hooklib/app.h"
#include "hooklib/rs232.h"

#include "sdvxhook/acio.h"
#include "sdvxhook/lcd.h"
#include "sdvxhook/gfx.h"

#include "util/cmdline.h"
#include "util/defs.h"
#include "util/log.h"

static bool my_dll_entry_init(char *sidcode, struct property_node *config);
static bool my_dll_entry_main(void);

static const irp_handler_t sdvxhook_handlers[] = {
    ac_io_bus_dispatch_irp,
    lcd_dispatch_irp,
};

static bool my_dll_entry_init(char *sidcode, struct property_node *config)
{
    bool ok;

    log_info("--- Begin sdvxhook dll_entry_init ---");

    ac_io_bus_init();

    log_info("Starting up SDVX IO backend");

    sdvx_io_set_loggers(
            log_body_misc,
            log_body_info,
            log_body_warning,
            log_body_fatal);

    ok = sdvx_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

    if (!ok) {
        goto sdvx_io_fail;
    }

    log_info("Starting up card reader backend");

    eam_io_set_loggers(
            log_body_misc,
            log_body_info,
            log_body_warning,
            log_body_fatal);

    ok = eam_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

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

    log_to_external(
            log_body_misc,
            log_body_info,
            log_body_warning,
            log_body_fatal);

    args_recover(&argc, &argv);

    for (i = 1 ; i < argc ; i++) {
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
    iohook_init(sdvxhook_handlers, lengthof(sdvxhook_handlers));
    rs232_hook_init();

    gfx_init();
    lcd_init();

    return TRUE;
}


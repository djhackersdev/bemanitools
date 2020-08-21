#include <windows.h>

#include <stdbool.h>

#include "bemanitools/ddrio.h"
#include "bemanitools/eamio.h"

#include "ddrhook/_com4.h"
#include "ddrhook/extio.h"
#include "ddrhook/gfx.h"
#include "ddrhook/master.h"
#include "ddrhook/p3io.h"
#include "ddrhook/spike.h"
#include "ddrhook/usbmem.h"

#include "hook/iohook.h"

#include "hooklib/app.h"
#include "hooklib/rs232.h"

#include "imports/avs.h"

#include "p3ioemu/emu.h"

#include "util/cmdline.h"
#include "util/defs.h"
#include "util/log.h"

static bool my_dll_entry_init(char *sidcode, struct property_node *param);
static bool my_dll_entry_main(void);

bool standard_def;
bool _15khz;

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    int argc;
    char **argv;
    bool com4;
    bool ok;
    int i;

    log_info("--- Begin ddrhook dll_entry_init ---");

    com4 = true;
    args_recover(&argc, &argv);

    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            continue;
        }

        switch (argv[i][1]) {
            case 'o':
                standard_def = true;

                break;

            case 'w':
                gfx_set_windowed();

                break;

            case 'u':
                /* Don't emulate P3IO COM4 and its downstream devices, use the
                   Windows COM4 port instead. */
                com4 = false;

                break;
        }
    }

    args_free(argc, argv);

    iohook_push_handler(p3io_emu_dispatch_irp);
    iohook_push_handler(extio_dispatch_irp);
    iohook_push_handler(spike_dispatch_irp);
    iohook_push_handler(usbmem_dispatch_irp);

    if (com4) {
        /* See ddrhook/p3io.c for details. */
        iohook_push_handler(com4_dispatch_irp);
    }

    rs232_hook_init();

    master_insert_hooks(NULL);
    p3io_ddr_init();
    extio_init();
    usbmem_init();
    spike_init();
    com4_init();

    log_info("Initializing DDR IO backend");

    ddr_io_set_loggers(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    ok = ddr_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

    if (!ok) {
        return false;
    }

    if (com4) {
        log_info("Initializing card reader backend");

        eam_io_set_loggers(
            log_body_misc, log_body_info, log_body_warning, log_body_fatal);

        ok = eam_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

        if (!ok) {
            return false;
        }
    }

    log_info("--- End ddrhook dll_entry_init ---");

    return app_hook_invoke_init(sidcode, param);
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    log_misc("Shutting down card reader backend");
    eam_io_fini();

    log_misc("Shutting down DDR IO backend");
    ddr_io_fini();

    com4_fini();
    spike_fini();
    usbmem_fini();
    extio_fini();
    p3io_emu_fini();

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

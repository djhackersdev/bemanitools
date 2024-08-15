#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "avs-ext/log.h"
#include "avs-ext/thread.h"

#include "hook/iohook.h"

#include "hooklib/app.h"
#include "hooklib/rs232.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/sdvx.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "sdvxhook/acio.h"
#include "sdvxhook/gfx.h"
#include "sdvxhook/lcd.h"

#include "util/cmdline.h"
#include "util/defs.h"

static bool my_dll_entry_init(char *sidcode, struct property_node *config);
static bool my_dll_entry_main(void);

static module_io_t *_sdvxhook_module_io_sdvx;
static module_io_t *_sdvxhook_module_io_eam;

static void _sdvxhook_io_sdvx_init(module_io_t **module)
{
    bt_io_sdvx_api_t api;

    module_io_ext_load_and_init(
        "sdvxio.dll", "bt_module_io_sdvx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_sdvx_api_set(&api);
}

static void _sdvxhook_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool my_dll_entry_init(char *sidcode, struct property_node *config)
{
    bool ok;

    log_info("--- Begin sdvxhook dll_entry_init ---");

    ac_io_bus_init();

    log_info("Starting up SDVX IO backend");

    _sdvxhook_io_sdvx_init(&_sdvxhook_module_io_sdvx);

    ok = bt_io_sdvx_init();

    if (!ok) {
        goto sdvx_io_fail;
    }

    log_info("Starting up card reader backend");

    _sdvxhook_io_eam_init(&_sdvxhook_module_io_eam);

    ok = bt_io_eam_init();

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
    bt_io_sdvx_fini();

    bt_io_sdvx_api_clear();
    module_io_free(&_sdvxhook_module_io_sdvx);

sdvx_io_fail:
    ac_io_bus_fini();

    return false;
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    log_info("Shutting down card reader backend");
    bt_io_eam_fini();

    bt_io_eam_api_clear();
    module_io_free(&_sdvxhook_module_io_eam);

    log_info("Shutting down SDVX IO backend");
    bt_io_sdvx_fini();

    bt_io_sdvx_api_clear();
    module_io_free(&_sdvxhook_module_io_sdvx);

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
    avs_ext_log_core_api_set();
    avs_ext_thread_core_api_set();

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

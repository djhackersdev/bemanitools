#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "hook/iohook.h"

#include "hooklib/app.h"
#include "hooklib/rs232.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/sdvx.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "sdvxhook/acio.h"
#include "sdvxhook/gfx.h"
#include "sdvxhook/lcd.h"

#include "util/cmdline.h"
#include "util/defs.h"

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

static bool
_sdvxhook_main_init(HMODULE game_module, const bt_core_config_t *config_)
{
    bool ok;
    int i;
    int argc;
    char **argv;

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

    log_info("--- Begin sdvxhook dll_entry_init ---");

    gfx_init();
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

    return true;

eam_io_fail:
    bt_io_sdvx_fini();

    bt_io_sdvx_api_clear();
    module_io_free(&_sdvxhook_module_io_sdvx);

sdvx_io_fail:
    ac_io_bus_fini();

    return false;
}

static void _sdvxhook_main_fini()
{
    log_info("Shutting down card reader backend");
    bt_io_eam_fini();

    bt_io_eam_api_clear();
    module_io_free(&_sdvxhook_module_io_eam);

    log_info("Shutting down SDVX IO backend");
    bt_io_sdvx_fini();

    bt_io_sdvx_api_clear();
    module_io_free(&_sdvxhook_module_io_sdvx);

    ac_io_bus_fini();
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

    api->v1.main_init = _sdvxhook_main_init;
    api->v1.main_fini = _sdvxhook_main_fini;
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    return TRUE;
}

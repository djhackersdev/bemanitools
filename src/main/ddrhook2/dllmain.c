#include <windows.h>

#include <stdbool.h>

#include "ddrhook-util/_com4.h"
#include "ddrhook-util/extio.h"
#include "ddrhook-util/gfx.h"
#include "ddrhook-util/p3io.h"
#include "ddrhook-util/spike.h"
#include "ddrhook-util/usbmem.h"

#include "ddrhook2/master.h"

#include "hook/iohook.h"

#include "hooklib/rs232.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/ddr.h"
#include "iface-io/eam.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "p3ioemu/emu.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/cmdline.h"
#include "util/defs.h"

bool standard_def;
bool _15khz;

static module_io_t *_ddrhook2_module_io_ddr;
static module_io_t *_ddrhook2_module_io_eam;

static void _ddrhook2_io_ddr_init(module_io_t **module)
{
    bt_io_ddr_api_t api;

    module_io_ext_load_and_init(
        "ddrio.dll", "bt_module_io_ddr_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_ddr_api_set(&api);
}

static void _ddrhook2_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool
_ddrhook2_main_init(HMODULE game_module, const bt_core_config_t *config_)
{
    int argc;
    char **argv;
    bool com4;
    bool ok;
    int i;
    char usbmem_data_path_p1[MAX_PATH] = "";
    char usbmem_data_path_p2[MAX_PATH] = "";
    bool usbmem_enabled = false;

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

            case 'a':
                /* Specify a USB memory path for P1 */
                if (i + 1 < argc) {
                    strcpy(usbmem_data_path_p1, argv[i + 1]);
                    usbmem_enabled = true;
                    i++; // Move forward one to skip the path parameter
                }

                break;

            case 'b':
                /* Specify a USB memory path for P2 */
                if (i + 1 < argc) {
                    strcpy(usbmem_data_path_p2, argv[i + 1]);
                    usbmem_enabled = true;
                    i++; // Move forward one to skip the path parameter
                }

                break;
        }
    }

    args_free(argc, argv);

    gfx_set_is_modern();

    iohook_push_handler(p3io_emu_dispatch_irp);
    iohook_push_handler(extio_dispatch_irp);
    iohook_push_handler(spike_dispatch_irp);
    iohook_push_handler(usbmem_dispatch_irp);

    if (com4) {
        /* See ddrhook2/p3io.c for details. */
        iohook_push_handler(com4_dispatch_irp);
    }

    rs232_hook_init();

    ddrhook2_master_insert_hooks(NULL);
    p3io_ddr_init();
    extio_init();
    usbmem_init(usbmem_data_path_p1, usbmem_data_path_p2, usbmem_enabled);
    spike_init();
    com4_init();

    log_info("Initializing DDR IO backend");

    _ddrhook2_io_ddr_init(&_ddrhook2_module_io_ddr);

    ok = bt_io_ddr_init();

    if (!ok) {
        return false;
    }

    if (com4) {
        log_info("Initializing card reader backend");

        _ddrhook2_io_eam_init(&_ddrhook2_module_io_eam);

        ok = bt_io_eam_init();

        if (!ok) {
            return false;
        }
    }

    log_info("--- End ddrhook dll_entry_init ---");

    return true;
}

static void _ddrhook2_main_fini()
{
    log_misc("Shutting down card reader backend");
    bt_io_eam_fini();
    bt_io_eam_api_clear();
    module_io_free(&_ddrhook2_module_io_eam);

    log_misc("Shutting down DDR IO backend");
    bt_io_ddr_fini();
    bt_io_ddr_api_clear();
    module_io_free(&_ddrhook2_module_io_ddr);

    com4_fini();
    spike_fini();
    usbmem_fini();
    extio_fini();
    p3io_emu_fini();
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

    api->v1.main_init = _ddrhook2_main_init;
    api->v1.main_fini = _ddrhook2_main_fini;
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    return TRUE;
}

#include <windows.h>

#include <stdbool.h>

#include "bsthook/acio.h"
#include "bsthook/gfx.h"
#include "bsthook/settings.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/bst.h"
#include "iface-io/eam.h"

#include "hook/iohook.h"

#include "hooklib/rs232.h"

#include "module/io-ext.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/cmdline.h"
#include "util/defs.h"

static module_io_t *_bsthook_module_io_bst;
static module_io_t *_bsthook_module_io_eam;

static void _bsthook_io_bst_init(module_io_t **module)
{
    bt_io_bst_api_t api;

    module_io_ext_load_and_init(
        "bstio.dll", "bt_module_io_bst_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_bst_api_set(&api);
}

static void _bsthook_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool
_bsthook_main_init(HMODULE game_module, const bt_core_config_t *config_)
{
    int i;
    int argc;
    char **argv;
    bool ok;

    args_recover(&argc, &argv);

    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            continue;
        }

        switch (argv[i][1]) {
            case 'w':
                gfx_set_windowed();

                break;
        }
    }

    args_free(argc, argv);

    iohook_push_handler(ac_io_bus_dispatch_irp);
    rs232_hook_init();

    gfx_init();
    settings_hook_init();

    log_info("--- Begin bsthook dll_entry_init ---");

    log_info("Starting up BeatStream IO backend");

    _bsthook_io_bst_init(&_bsthook_module_io_bst);

    ok = bt_io_bst_init();

    if (!ok) {
        goto bst_io_fail;
    }

    _bsthook_io_eam_init(&_bsthook_module_io_eam);

    ok = bt_io_eam_init();

    if (!ok) {
        goto eam_io_fail;
    }

    ac_io_bus_init();

    log_info("---  End  bsthook dll_entry_init ---");

    return true;

eam_io_fail:
    bt_io_bst_fini();

    bt_io_bst_api_clear();
    module_io_free(&_bsthook_module_io_bst);

bst_io_fail:

    return false;
}

static void _bsthook_main_fini()
{
    log_info("Shutting down card reader backend");
    bt_io_eam_fini();

    bt_io_eam_api_clear();
    module_io_free(&_bsthook_module_io_eam);

    log_info("Shutting down BeatStream IO backend");
    bt_io_bst_fini();

    bt_io_bst_api_clear();
    module_io_free(&_bsthook_module_io_bst);

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

    api->v1.main_init = _bsthook_main_init;
    api->v1.main_fini = _bsthook_main_fini;
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    return TRUE;
}

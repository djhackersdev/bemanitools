#include <windows.h>

#include <stdbool.h>

#include "avs/core-interop.h"

#include "btsdk/core/log.h"
#include "btsdk/core/thread.h"

#include "btapi/io/bstio.h"
#include "btapi/io/eamio.h"

#include "hook/iohook.h"

#include "hooklib/app.h"
#include "hooklib/rs232.h"

#include "imports/avs.h"

#include "bsthook/acio.h"
#include "bsthook/gfx.h"
#include "bsthook/settings.h"

#include "util/cmdline.h"
#include "util/defs.h"

static bool my_dll_entry_init(char *sidcode, struct property_node *config);
static bool my_dll_entry_main(void);

static bool my_dll_entry_init(char *sidcode, struct property_node *config)
{
    bool ok;

    log_info("--- Begin bsthook dll_entry_init ---");

    ac_io_bus_init();

    log_info("Starting up BeatStream IO backend");

    // TODO use btsdk once migrated
    bst_io_set_loggers(_bt_core_log_impl.misc, _bt_core_log_impl.info, _bt_core_log_impl.warning, _bt_core_log_impl.fatal);

    ok = bst_io_init(
        bt_core_thread_create,
        bt_core_thread_join,
        bt_core_thread_destroy);

    if (!ok) {
        goto bst_io_fail;
    }

    // TODO use btsdk once migrated
    eam_io_set_loggers(_bt_core_log_impl.misc, _bt_core_log_impl.info, _bt_core_log_impl.warning, _bt_core_log_impl.fatal);

    ok = eam_io_init(
        bt_core_thread_create,
        bt_core_thread_join,
        bt_core_thread_destroy);

    if (!ok) {
        goto eam_io_fail;
    }

    log_info("---  End  bsthook dll_entry_init ---");

    return app_hook_invoke_init(sidcode, config);

eam_io_fail:
    bst_io_fini();

bst_io_fail:
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
    bst_io_fini();

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

    // TODO use btsdk once migrated
    // Use AVS APIs
    // avs_core_interop_log_avs_impl_set();
    // avs_core_interop_thread_avs_impl_set();

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

    app_hook_init(my_dll_entry_init, my_dll_entry_main);
    iohook_push_handler(ac_io_bus_dispatch_irp);
    rs232_hook_init();

    gfx_init();
    settings_hook_init();

    return TRUE;
}

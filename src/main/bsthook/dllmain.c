#include <windows.h>

#include <stdbool.h>

#include "avs-util/core-interop.h"

#include "bemanitools/bstio.h"
#include "bemanitools/eamio.h"

#include "core/log.h"
#include "core/thread.h"

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

    core_log_impl_assign(bst_io_set_loggers);

    ok = bst_io_init(
        core_thread_create_impl_get(),
        core_thread_join_impl_get(),
        core_thread_destroy_impl_get());

    if (!ok) {
        goto bst_io_fail;
    }

    core_log_impl_assign(eam_io_set_loggers);

    ok = eam_io_init(
        core_thread_create_impl_get(),
        core_thread_join_impl_get(),
        core_thread_destroy_impl_get());

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

    // Use AVS APIs
    avs_util_core_interop_thread_avs_impl_set();
    avs_util_core_interop_log_avs_impl_set();

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

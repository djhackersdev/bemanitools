#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "avs-util/core-interop.h"

#include "bemanitools/eamio.h"
#include "bemanitools/jbio.h"

#include "core/log.h"
#include "core/thread.h"

#include "hook/iohook.h"
#include "hook/table.h"

#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "imports/avs.h"

#include "jbhook3/gfx.h"
#include "jbhook3/options.h"

#include "jbhook-util/acio.h"
#include "jbhook-util/eamuse.h"
#include "jbhook-util/p4io.h"

#include "p4ioemu/device.h"
#include "p4ioemu/setupapi.h"

#include "security/id.h"

#include "util/defs.h"

static struct options options;

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    bool eam_io_ok;
    bool jb_io_ok;

    eam_io_ok = false;
    jb_io_ok = false;

    log_info("--- Begin jbhook dll_entry_init ---");

    iohook_push_handler(p4ioemu_dispatch_irp);
    iohook_push_handler(jbhook_util_ac_io_port_dispatch_irp);

    jbhook3_gfx_init();

    if (options.windowed) {
        jbhook3_gfx_set_windowed();
    }

    if (options.show_cursor) {
        jbhook3_gfx_set_show_cursor();
    }

    if (!options.disable_p4ioemu) {
        log_info("Starting up jubeat IO backend");

        core_log_impl_assign(jb_io_set_loggers);

        jb_io_ok = jb_io_init(
            core_thread_create_impl_get(),
            core_thread_join_impl_get(),
            core_thread_destroy_impl_get());

        if (!jb_io_ok) {
            goto fail;
        }

        hook_setupapi_init(&p4ioemu_setupapi_data);
        p4ioemu_init(jbhook_p4io_init());
    }

    if (!options.disable_cardemu) {
        log_info("Starting up card reader backend");

        core_log_impl_assign(eam_io_set_loggers);

        eam_io_ok = eam_io_init(
            core_thread_create_impl_get(),
            core_thread_join_impl_get(),
            core_thread_destroy_impl_get());

        if (!eam_io_ok) {
            goto fail;
        }

        rs232_hook_init();
        jbhook_util_ac_io_port_init(L"COM2");
        jbhook_util_ac_io_set_iccb();
    }

    log_info("---  End  jbhook dll_entry_init ---");

    bool ret = app_hook_invoke_init(sidcode, param);

    return ret;

fail:
    if (eam_io_ok) {
        eam_io_fini();
    }

    if (jb_io_ok) {
        jb_io_fini();
    }

    return false;
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    log_info("Shutting down card reader backend");
    eam_io_fini();

    log_info("Shutting down Jubeat IO backend");
    jb_io_fini();

    if (!options.disable_cardemu) {
        jbhook_util_ac_io_port_fini();
    }

    if (!options.disable_p4ioemu) {
        p4ioemu_fini();
    }

    options_fini(&options);

    return result;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    // Use AVS APIs
    avs_util_core_interop_thread_avs_impl_set();
    avs_util_core_interop_log_avs_impl_set();

    options_init_from_cmdline(&options);

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    if (!options.disable_adapteremu) {
        adapter_hook_init();
    }

    jbhook_util_eamuse_hook_init();

    return TRUE;
}

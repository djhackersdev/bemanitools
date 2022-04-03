#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/jbio.h"

#include "hook/iohook.h"

#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "imports/avs.h"

#include "jbhook3/options.h"

#include "jbhook-util/acio.h"
#include "jbhook-util/eamuse.h"
#include "jbhook-util/p4io.h"

#include "p4ioemu/device.h"
#include "p4ioemu/setupapi.h"

#include "security/id.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/thread.h"

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

    if (!options.disable_p4ioemu) {
        log_info("Starting up jubeat IO backend");

        jb_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        jb_io_ok =
            jb_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

        if (!jb_io_ok) {
            goto fail;
        }

        hook_setupapi_init(&p4ioemu_setupapi_data);
        p4ioemu_init(jbhook_p4io_init());
    }

    if (!options.disable_cardemu) {
        log_info("Starting up card reader backend");

        eam_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        eam_io_ok =
            eam_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

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

    log_to_external(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    options_init_from_cmdline(&options);

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    if (!options.disable_adapteremu) {
        adapter_hook_init();
    }

    jbhook_util_eamuse_hook_init();

    return TRUE;
}

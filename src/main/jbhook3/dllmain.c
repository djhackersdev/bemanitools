#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hook/iohook.h"
#include "hook/table.h"

#include "hooklib/adapter.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/jb.h"

#include "jbhook3/gfx.h"
#include "jbhook3/options.h"

#include "jbhook-util/acio.h"
#include "jbhook-util/eamuse.h"
#include "jbhook-util/p4io.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "p4ioemu/device.h"
#include "p4ioemu/setupapi.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "security/id.h"

#include "util/defs.h"

static struct options options;
static module_io_t *jbhook_module_io_jb;
static module_io_t *jbhook_module_io_eam;

static void _jbhook3_io_jb_init(module_io_t **module)
{
    bt_io_jb_api_t api;

    module_io_ext_load_and_init("jbio.dll", "bt_module_io_jb_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_jb_api_set(&api);
}

static void _jbhook3_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool
_jbhook3_main_init(HMODULE game_module, const bt_core_config_t *config_)
{
    bool eam_io_ok;
    bool jb_io_ok;

    eam_io_ok = false;
    jb_io_ok = false;

    log_info("--- Begin jbhook dll_entry_init ---");

    options_init_from_cmdline(&options);

    if (!options.disable_adapteremu) {
        adapter_hook_init();
    }

    jbhook_util_eamuse_hook_init();

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

        _jbhook3_io_jb_init(&jbhook_module_io_jb);

        jb_io_ok = bt_io_jb_init();

        if (!jb_io_ok) {
            goto fail;
        }

        hook_setupapi_init(&p4ioemu_setupapi_data);
        p4ioemu_init(jbhook_p4io_init());
    }

    if (!options.disable_cardemu) {
        log_info("Starting up card reader backend");

        _jbhook3_io_eam_init(&jbhook_module_io_eam);

        eam_io_ok = bt_io_eam_init();

        if (!eam_io_ok) {
            goto fail;
        }

        rs232_hook_init();
        jbhook_util_ac_io_port_init(L"COM2");
        jbhook_util_ac_io_set_iccb();
    }

    log_info("---  End  jbhook dll_entry_init ---");

    return true;

fail:
    if (eam_io_ok) {
        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&jbhook_module_io_eam);
    }

    if (jb_io_ok) {
        bt_io_jb_fini();

        bt_io_jb_api_clear();
        module_io_free(&jbhook_module_io_jb);
    }

    return false;
}

static void _jbhook3_main_fini()
{
    if (!options.disable_cardemu) {
        log_info("Shutting down card reader backend");

        jbhook_util_ac_io_port_fini();
        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&jbhook_module_io_eam);
    }

    if (!options.disable_p4ioemu) {
        log_info("Shutting down Jubeat IO backend");

        p4ioemu_fini();

        bt_io_jb_fini();

        bt_io_jb_api_clear();
        module_io_free(&jbhook_module_io_jb);
    }

    options_fini(&options);
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

    api->v1.main_init = _jbhook3_main_init;
    api->v1.main_fini = _jbhook3_main_fini;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    return TRUE;
}

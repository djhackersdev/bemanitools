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

#include "jbhook2/options.h"

#include "jbhook-util/acio.h"
#include "jbhook-util/eamuse.h"

#include "jbhook-util-p3io/gfx.h"
#include "jbhook-util-p3io/p3io.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "p3ioemu/devmgr.h"
#include "p3ioemu/emu.h"

#include "p4ioemu/device.h"
#include "p4ioemu/setupapi.h"

#include "security/id.h"
#include "security/mcode.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/defs.h"

static struct options options;
static module_io_t *jbhook_module_io_jb;
static module_io_t *jbhook_module_io_eam;

static HWND CDECL
my_mwindow_create(HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);
static HWND(CDECL *real_mwindow_create)(
    HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "mwindow_create",
        .patch = my_mwindow_create,
        .link = (void **) &real_mwindow_create,
    },
};

static HWND CDECL my_mwindow_create(
    HINSTANCE hinstance,
    void *callback,
    const char *window_title,
    DWORD window_width,
    DWORD window_height,
    BOOL fullscreen)
{
    log_info("-------------------------------------------------------------");
    log_info("---------------- Begin jbhook mwindow_create ----------------");
    log_info("-------------------------------------------------------------");

    if (options.vertical) {
        DWORD tmp = window_width;
        window_width = window_height;
        window_height = tmp;
    }

    if (options.show_cursor) {
        ShowCursor(TRUE);
    }

    fullscreen = !options.windowed;

    return real_mwindow_create(
        hinstance,
        callback,
        window_title,
        window_width,
        window_height,
        fullscreen);
}

static void _jbhook2_io_jb_init(module_io_t **module)
{
    bt_io_jb_api_t api;

    module_io_ext_load_and_init("jbio.dll", "bt_module_io_jb_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_jb_api_set(&api);
}

static void _jbhook2_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool
_jbhook2_main_init(HMODULE game_module, const bt_core_config_t *config_)
{
    bool eam_io_ok;
    bool jb_io_ok;

    eam_io_ok = false;
    jb_io_ok = false;

    log_info("--- Begin jbhook dll_entry_init ---");

    char *sidcode = NULL; // TODO this isn't propagated with the new api anymore, what to do?

    log_assert(sidcode != NULL);

    options_init_from_cmdline(&options);

    hook_table_apply(
        NULL, "mwindow.dll", init_hook_syms, lengthof(init_hook_syms));

    if (!options.disable_adapteremu) {
        adapter_hook_init();
    }

    jbhook_util_eamuse_hook_init();

    if (options.vertical) {
        jbhook_util_gfx_install_vertical_hooks();
    }

    if (!options.disable_p3ioemu) {
        iohook_push_handler(p3io_emu_dispatch_irp);
        p3io_setupapi_insert_hooks(NULL);

        // pcbid and eamid are only used here for sec check, the ones used for
        // network access are taken from ea3-config.xml.
        // When booting knit append (and on no other version), the code must
        // actually match, so we use the one from ea3-config as well.
        // Example sidcode format: `K44JBA2012072301`
        struct security_mcode security_mcode_j44 = {
            .header = SECURITY_MCODE_HEADER,
            .unkn = SECURITY_MCODE_UNKN_C,
            .game = {sidcode[0], sidcode[1], sidcode[2]},
            .region = sidcode[3],
            .cabinet = sidcode[4],
            .revision = sidcode[5],
        };

        jbhook_util_p3io_init(
            &security_mcode_j44, &security_id_default, &security_id_default);

        log_info("Starting up jubeat IO backend");

        _jbhook2_io_jb_init(&jbhook_module_io_jb);

        jb_io_ok = bt_io_jb_init();

        if (!jb_io_ok) {
            goto fail;
        }
    }

    if (!options.disable_cardemu) {
        iohook_push_handler(jbhook_util_ac_io_port_dispatch_irp);
        rs232_hook_init();
        jbhook_util_ac_io_port_init(L"COM1");
        jbhook_util_ac_io_set_iccb();

        log_info("Starting up card reader backend");

        _jbhook2_io_eam_init(&jbhook_module_io_eam);

        eam_io_ok = bt_io_eam_init();

        if (!eam_io_ok) {
            goto fail;
        }
    }

    log_info("---  End  jbhook dll_entry_init ---");

    char *sidcode_in_ea3_config = strdup(sidcode);

    // TODO the following needs the sidcode back-propagation, figure something out
    // bool ret = app_hook_invoke_init(sidcode, param);

    // If the game is append, the mcode `cabinet` is forced to C. This is bad if
    // p3io was configured to respond as A! Help the user help themsevles...
    if (strcmp(sidcode_in_ea3_config, sidcode) != 0) {
        log_warning(
            "sidcode changed after running game DLL init (%s -> %s)",
            sidcode_in_ea3_config,
            sidcode);
        log_warning(
            "This will trigger a security error. Modify ea3-config.xml <soft> "
            "section to match the second value!");

        // abort the boot
        return false;
    }

    free(sidcode_in_ea3_config);

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

static void _jbhook2_main_fini()
{
    if (!options.disable_cardemu) {
        log_info("Shutting down card reader backend");

        jbhook_util_ac_io_port_fini();
        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&jbhook_module_io_eam);
    }

    if (!options.disable_p3ioemu) {
        log_info("Shutting down Jubeat IO backend");

        jbhook_util_p3io_fini();

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

    api->v1.main_init = _jbhook2_main_init;
    api->v1.main_fini = _jbhook2_main_fini;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    return TRUE;
}

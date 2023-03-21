#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/jbio.h"

#include "cconfig/cconfig-hook.h"

#include "hook/table.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/rs232.h"

#include "jbhook1/avs-boot.h"
#include "jbhook1/config-eamuse.h"
#include "jbhook1/config-gfx.h"
#include "jbhook1/config-security.h"
#include "jbhook1/log-gftools.h"

#include "jbhook-util/acio.h"
#include "jbhook-util/eamuse.h"

#include "jbhook-util-p3io/gfx.h"
#include "jbhook-util-p3io/mixer.h"
#include "jbhook-util-p3io/p3io.h"

#include "p3ioemu/devmgr.h"
#include "p3ioemu/emu.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/thread.h"

#define JBHOOK1_INFO_HEADER \
    "jbhook1 for jubeat"    \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define JBHOOK1_CMD_USAGE \
    "Usage: inject.exe jbhook1.dll <jubeat.exe> [options...]"

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

/**
 * This seems to be a good entry point to intercept before the game calls
 * anything important (very close to the start of WinMain).
 */
static HWND CDECL my_mwindow_create(
    HINSTANCE hinstance,
    void *callback,
    const char *window_title,
    DWORD window_width,
    DWORD window_height,
    BOOL fullscreen)
{
    struct cconfig *config;

    struct jbhook1_config_gfx config_gfx;
    struct jbhook1_config_eamuse config_eamuse;
    struct jbhook1_config_security config_security;

    log_info("-------------------------------------------------------------");
    log_info("---------------- Begin jbhook mwindow_create ----------------");
    log_info("-------------------------------------------------------------");

    jbhook_util_mixer_hook_init();

    config = cconfig_init();

    jbhook1_config_gfx_init(config);
    jbhook1_config_eamuse_init(config);
    jbhook1_config_security_init(config);

    if (!cconfig_hook_config_init(
            config,
            JBHOOK1_INFO_HEADER "\n" JBHOOK1_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    jbhook1_config_gfx_get(&config_gfx, config);
    jbhook1_config_eamuse_get(&config_eamuse, config);
    jbhook1_config_security_get(&config_security, config);

    cconfig_finit(config);

    log_info(JBHOOK1_INFO_HEADER);
    log_info("Initializing jbhook...");

    jbhook1_avs_boot_init();
    jbhook1_avs_boot_set_eamuse_addr(&config_eamuse.server);
    jbhook1_log_gftools_init();

    fullscreen = !config_gfx.windowed;

    if (config_gfx.vertical) {
        DWORD tmp = window_width;
        window_width = window_height;
        window_height = tmp;

        jbhook_util_gfx_install_vertical_hooks();
    }

    log_info("Starting up jubeat IO backend");

    jb_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!jb_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing jb IO backend failed");
    }

    log_info("Starting up card reader backend");

    eam_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!eam_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing card reader backend failed");
    }

    jbhook_util_eamuse_hook_init();

    iohook_push_handler(p3io_emu_dispatch_irp);
    iohook_push_handler(jbhook_util_ac_io_port_dispatch_irp);

    rs232_hook_init();
    jbhook_util_ac_io_port_init(L"COM1");
    jbhook_util_ac_io_set_iccb();

    p3io_setupapi_insert_hooks(NULL);
    jbhook_util_p3io_init(
        &config_security.mcode, &config_eamuse.pcbid, &config_eamuse.eamid);

    log_info("-------------------------------------------------------------");
    log_info("----------------- End jbhook mwindow_create -----------------");
    log_info("-------------------------------------------------------------");

    return real_mwindow_create(
        hinstance,
        callback,
        window_title,
        window_width,
        window_height,
        fullscreen);
}

/**
 * Hook library for jubeat (1)
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        log_to_writer(log_writer_debug, NULL);

        /* Bootstrap hook for further init tasks (see above) */

        hook_table_apply(
            NULL, "mwindow.dll", init_hook_syms, lengthof(init_hook_syms));

        /* Actual hooks for game specific stuff */

        acp_hook_init();
        adapter_hook_init();
    }

    return TRUE;
}

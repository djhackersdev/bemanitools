#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/jbio.h"

#include "cconfig/cconfig-hook.h"

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-debug.h"
#include "core/log.h"
#include "core/thread-crt-ext.h"
#include "core/thread-crt.h"
#include "core/thread.h"

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

#define JBHOOK1_INFO_HEADER \
    "jbhook1 for jubeat"    \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define JBHOOK1_CMD_USAGE \
    "Usage: inject.exe jbhook1.dll <jubeat.exe> [options...]"

static HWND CDECL
my_mwindow_create(HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);
static HWND(CDECL *real_mwindow_create)(
    HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);

static BOOL STDCALL my_CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);
static BOOL(STDCALL *real_CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "mwindow_create",
        .patch = my_mwindow_create,
        .link = (void **) &real_mwindow_create,
    },
};

static const struct hook_symbol kernel32_hook_syms[] = {
    {
        .name = "CreateProcessA",
        .patch = my_CreateProcessA,
        .link = (void **) &real_CreateProcessA,
    },
};

// so our CreateProcessA hook can check
static bool vertical;

static void _jbhook1_log_init()
{
    core_log_bt_ext_impl_set();
    core_log_bt_ext_init_with_debug();
    // TODO change log level support
    core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);
}

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
    vertical = config_gfx.vertical;

    if (config_gfx.vertical) {
        DWORD tmp = window_width;
        window_width = window_height;
        window_height = tmp;

        jbhook_util_gfx_install_vertical_hooks();
    }

    log_info("Starting up jubeat IO backend");

    core_log_impl_assign(jb_io_set_loggers);

    if (!jb_io_init(
            core_thread_create_impl_get(),
            core_thread_join_impl_get(),
            core_thread_destroy_impl_get())) {
        log_fatal("Initializing jb IO backend failed");
    }

    log_info("Starting up card reader backend");

    core_log_impl_assign(eam_io_set_loggers);

    if (!eam_io_init(
            core_thread_create_impl_get(),
            core_thread_join_impl_get(),
            core_thread_destroy_impl_get())) {
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
        // TODO why not use AVS threads?
        core_thread_crt_ext_impl_set();

        // TODO init debug logging but with avs available? why not use avs
        // logging?
        _jbhook1_log_init();

        /* Bootstrap hook for further init tasks (see above) */

        hook_table_apply(
            NULL, "mwindow.dll", init_hook_syms, lengthof(init_hook_syms));

        /* Actual hooks for game specific stuff */

        hook_table_apply(
            NULL,
            "kernel32.dll",
            kernel32_hook_syms,
            lengthof(kernel32_hook_syms));

        acp_hook_init();
        adapter_hook_init();
    }

    return TRUE;
}

static BOOL STDCALL my_CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    LPSTR rot_arg;

    // de-rotate the error window if needed. In theory this should parse the
    // commandline properly, in practice this works on all the .exe jubeats.
    if (vertical && lpCommandLine) {
        if ((rot_arg = strstr(lpCommandLine, " -R90 "))) { // jubeat 01
            memcpy(rot_arg, " -R0  ", strlen(" -R0  "));
        } else if ((rot_arg =
                        strstr(lpCommandLine, " -rot:90 "))) { // jubeat 02
            memcpy(rot_arg, " -rot:0  ", strlen(" -rot:0  "));
        }
    }

    return real_CreateProcessA(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation);
}

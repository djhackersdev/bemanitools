#include <windows.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "ezusb-emu/desc.h"
#include "ezusb-emu/device.h"

#include "ezusb-iidx-emu/msg.h"
#include "ezusb-iidx-emu/node-security-plug.h"
#include "ezusb-iidx-emu/node-serial.h"
#include "ezusb-iidx-emu/nodes.h"

#include "hook/iohook.h"
#include "hook/table.h"

#include "hooklib/adapter.h"
#include "hooklib/acp.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/chart-patch.h"
#include "iidxhook-util/clock.h"
#include "iidxhook-util/config-eamuse.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-misc.h"
#include "iidxhook-util/config-sec.h"
#include "iidxhook-util/d3d8.h"
#include "iidxhook-util/eamuse.h"
#include "iidxhook-util/effector.h"
#include "iidxhook-util/settings.h"

#include "iidxhook2/config-iidxhook2.h"

#include "util/log.h"
#include "util/thread.h"

#define IIDXHOOK2_INFO_HEADER \
    "iidxhook for DistorteD" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK2_CMD_USAGE \
    "Usage: inject.exe iidxhook2.dll <bm2dx.exe> [options...]"

static const irp_handler_t iidxhook_handlers[] = {
    ezusb_emu_device_dispatch_irp,
    iidxhook_util_acio_dispatch_irp,
    iidxhook_util_chart_patch_dispatch_irp,
    settings_hook_dispatch_irp,
};

static HANDLE STDCALL my_OpenProcess(DWORD, BOOL, DWORD);
static HANDLE (STDCALL *real_OpenProcess)(DWORD, BOOL, DWORD);
static bool iidxhook_init_check;

static const struct hook_symbol init_hook_syms[] = {
    {
        .name       = "OpenProcess",
        .patch      = my_OpenProcess,
        .link       = (void **) &real_OpenProcess
    },
};

/**
 * This seems to be a good entry point to intercept
 * before the game calls anything important
 */
HANDLE STDCALL my_OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle,
        DWORD dwProcessId)
{
    struct cconfig* config;

    struct iidxhook_util_config_eamuse config_eamuse;
    struct iidxhook_config_gfx config_gfx;
    struct iidxhook_config_iidxhook2 config_iidxhook2;
    struct iidxhook_config_misc config_misc;
    struct iidxhook_config_sec config_sec;

    if (iidxhook_init_check) {
        goto skip;
    }

    iidxhook_init_check = true;

    log_info("-------------------------------------------------------------");
    log_info("--------------- Begin iidxhook my_OpenProcess ---------------");
    log_info("-------------------------------------------------------------");

    config = cconfig_init();

    iidxhook_util_config_eamuse_init(config);
    iidxhook_config_gfx_init(config);
    iidxhook_config_iidxhook2_init(config);
    iidxhook_config_misc_init(config);
    iidxhook_config_sec_init(config);

    if (!cconfig_hook_config_init(config, IIDXHOOK2_INFO_HEADER "\n" IIDXHOOK2_CMD_USAGE, CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    iidxhook_util_config_eamuse_get(&config_eamuse, config);
    iidxhook_config_gfx_get(&config_gfx, config);
    iidxhook_config_iidxhook2_get(&config_iidxhook2, config);
    iidxhook_config_misc_get(&config_misc, config);
    iidxhook_config_sec_get(&config_sec, config);

    cconfig_finit(config);

    log_info(IIDXHOOK2_INFO_HEADER);
    log_info("Initializing iidxhook...");

    /* Round plug security */

    ezusb_iidx_emu_node_security_plug_set_boot_version(
        &config_sec.boot_version);
    ezusb_iidx_emu_node_security_plug_set_boot_seeds(config_sec.boot_seeds);
    ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
        &config_sec.black_plug_mcode);
    ezusb_iidx_emu_node_security_plug_set_pcbid(&config_eamuse.pcbid);

    /* eAmusement server IP */

    eamuse_set_addr(&config_eamuse.server);
    eamuse_check_connection();

    /* Patch rteffect.dll calls */

    if (config_misc.rteffect_stub) {
        effector_hook_init();
    }

    /* Direct3D and USER32 hooks */

    /* There are a few features that cannot be implemented using d3d8, 
       e.g. GPU based upscaling with filtering. When using d3d8to9, allow
       using the d3d9 hooks instead because the game is running on d3d9. */
    if (config_iidxhook2.use_d3d9_hooks) {
        log_warning("d3d9 hook modules enabled. Requires d3d8to9!");

        d3d9_hook_init();

        if (config_gfx.bgvideo_uv_fix) {
            /* Red, HS, DistorteD, only */
            d3d9_iidx_fix_stretched_bg_videos();
        }

        if (config_gfx.windowed) {
            d3d9_set_windowed(config_gfx.framed, config_gfx.window_width,
                config_gfx.window_height);
        }

        if (config_gfx.frame_rate_limit > 0) {
            d3d9_set_frame_rate_limit(config_gfx.frame_rate_limit);
        }

        if (config_gfx.monitor_check == 0) {
            log_info("Auto monitor check enabled");
            d3d9_enable_monitor_check(iidxhook_util_chart_patch_set_refresh_rate);
            iidxhook_util_chart_patch_init(
                IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_9_TO_13);
        } else if (config_gfx.monitor_check > 0) {
            log_info("Manual monitor check, resulting refresh rate: %f", 
                config_gfx.monitor_check);
            iidxhook_util_chart_patch_init(
                IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_9_TO_13);
            iidxhook_util_chart_patch_set_refresh_rate(config_gfx.monitor_check);
        }

        /* Fix DistorteD 3D background on music select screen */

        if (config_iidxhook2.distorted_ms_bg_fix) {
            d3d9_iidx_fix_13_song_select_bg();
        }

        if (config_gfx.scale_back_buffer_width > 0 && config_gfx.scale_back_buffer_height > 0) {
            d3d9_scale_back_buffer(config_gfx.scale_back_buffer_width, config_gfx.scale_back_buffer_height,
                config_gfx.scale_back_buffer_filter);
        }
    } else {
        d3d8_hook_init();

        if (config_gfx.bgvideo_uv_fix) {
            /* Red, HS, DistorteD, only */
            d3d8_iidx_fix_stretched_bg_videos();
        }

        if (config_gfx.windowed) {
            d3d8_set_windowed(config_gfx.framed, config_gfx.window_width,
                config_gfx.window_height);
        }

        if (config_gfx.frame_rate_limit > 0) {
            d3d8_set_frame_rate_limit(config_gfx.frame_rate_limit);
        }

        if (config_gfx.monitor_check == 0) {
            log_info("Auto monitor check enabled");
            d3d8_enable_monitor_check(iidxhook_util_chart_patch_set_refresh_rate);
            iidxhook_util_chart_patch_init(
                IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_9_TO_13);
        } else if (config_gfx.monitor_check > 0) {
            log_info("Manual monitor check, resulting refresh rate: %f", 
                config_gfx.monitor_check);
            iidxhook_util_chart_patch_init(
                IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_9_TO_13);
            iidxhook_util_chart_patch_set_refresh_rate(config_gfx.monitor_check);
        }

        /* Fix DistorteD 3D background on music select screen */

        if (config_iidxhook2.distorted_ms_bg_fix) {
            d3d8_iidx_fix_13_song_select_bg();
        }
    }

    /* Disable operator menu clock setting system clock time */

    if (config_misc.disable_clock_set) {
        iidxhook_util_clock_hook_init();
    }

    /* Start up IIDXIO.DLL */

    log_info("Starting IIDX IO backend");
    iidx_io_set_loggers(log_impl_misc, log_impl_info, log_impl_warning,
            log_impl_fatal);

    if (!iidx_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing IIDX IO backend failed");
    }

    /* Start up EAMIO.DLL */

    log_misc("Initializing card reader backend");
    eam_io_set_loggers(log_impl_misc, log_impl_info, log_impl_warning,
        log_impl_fatal);

    if (!eam_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing card reader backend failed");
    }

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_init(iidxhook_handlers, lengthof(iidxhook_handlers));

    hook_setupapi_init(&ezusb_emu_desc_device.setupapi);
    ezusb_emu_device_hook_init(ezusb_iidx_emu_msg_init());

    /* Card reader emulation, same issue with hooking as IO emulation */
    rs232_hook_init();

    iidxhook_util_acio_init(true);

    log_info("-------------------------------------------------------------");
    log_info("---------------- End iidxhook my_OpenProcess ----------------");
    log_info("-------------------------------------------------------------");

skip:
    return real_OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
}

/**
 * Hook library for DistorteD
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
#ifdef DEBUG_HOOKING
        FILE* file = fopen("iidxhook.dllmain.log", "w+");
        log_to_writer(log_writer_file, file);
#else
        log_to_writer(log_writer_null, NULL);
#endif

        /* Bootstrap hook for further init tasks (see above) */

        hook_table_apply(
                NULL,
                "kernel32.dll",
                init_hook_syms,
                lengthof(init_hook_syms));

        /* Actual hooks for game specific stuff */

        acp_hook_init();
        adapter_hook_init();
        eamuse_hook_init();
        settings_hook_init();

#ifdef DEBUG_HOOKING
        fflush(file);
        fclose(file);
#endif

        /* Logging to file and other destinations is handled by inject */
        log_to_writer(log_writer_debug, NULL);
    }

    return TRUE;
}


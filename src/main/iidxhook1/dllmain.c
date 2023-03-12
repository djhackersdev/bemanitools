#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "ezusb-emu/desc.h"
#include "ezusb-emu/device.h"
#include "ezusb-emu/node-security-plug.h"

#include "ezusb-iidx-emu/msg.h"
#include "ezusb-iidx-emu/node-serial.h"
#include "ezusb-iidx-emu/nodes.h"

#include "hook/d3d9.h"
#include "hook/iohook.h"
#include "hook/table.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/setupapi.h"

#include "iidxhook-util/chart-patch.h"
#include "iidxhook-util/clock.h"
#include "iidxhook-util/config-eamuse.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-misc.h"
#include "iidxhook-util/config-sec.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/eamuse.h"
#include "iidxhook-util/effector.h"
#include "iidxhook-util/settings.h"

#include "iidxhook1/config-iidxhook1.h"
#include "iidxhook1/log-ezusb.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/thread.h"

#define IIDXHOOK1_INFO_HEADER                               \
    "iidxhook for 9th Style, 10th Style, RED and HAPPY SKY" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK1_CMD_USAGE \
    "Usage: inject.exe iidxhook1.dll <bm2dx.exe> [options...]"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static HANDLE STDCALL my_OpenProcess(DWORD, BOOL, DWORD);
static HANDLE(STDCALL *real_OpenProcess)(DWORD, BOOL, DWORD);

static bool iidxhook_init_check;

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "OpenProcess",
        .patch = my_OpenProcess,
        .link = (void **) &real_OpenProcess,
    },
};

static void iidxhook1_setup_d3d9_hooks(
    const struct iidxhook_config_gfx *config_gfx,
    const struct iidxhook_config_iidxhook1 *config_iidxhook1)
{
    struct iidxhook_util_d3d9_config d3d9_config;

    log_warning(
        "d3d8 hook module deprecated, using d3d9 hook modules requiring "
        "d3d8to9 to work!");

    iidxhook_util_d3d9_init_config(&d3d9_config);

    d3d9_config.windowed = config_gfx->windowed;
    d3d9_config.framed = config_gfx->framed;
    d3d9_config.override_window_width = config_gfx->window_width;
    d3d9_config.override_window_height = config_gfx->window_height;
    d3d9_config.framerate_limit = config_gfx->frame_rate_limit;
    d3d9_config.iidx12_fix_song_select_bg =
        config_iidxhook1->happy_sky_ms_bg_fix;
    d3d9_config.iidx11_to_17_fix_uvs_bg_videos = config_gfx->bgvideo_uv_fix;
    d3d9_config.scale_back_buffer_width = config_gfx->scale_back_buffer_width;
    d3d9_config.scale_back_buffer_height = config_gfx->scale_back_buffer_height;
    d3d9_config.scale_back_buffer_filter = config_gfx->scale_back_buffer_filter;
    d3d9_config.forced_refresh_rate = config_gfx->forced_refresh_rate;
    d3d9_config.device_adapter = config_gfx->device_adapter;

    if (config_gfx->monitor_check == 0) {
        log_info("Auto monitor check enabled");

        d3d9_config.iidx09_to_19_monitor_check_cb =
            iidxhook_util_chart_patch_set_refresh_rate;
        iidxhook_util_chart_patch_init(
            IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_9_TO_13);
    } else if (config_gfx->monitor_check > 0) {
        log_info(
            "Manual monitor check, resulting refresh rate: %f",
            config_gfx->monitor_check);

        iidxhook_util_chart_patch_init(
            IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_9_TO_13);
        iidxhook_util_chart_patch_set_refresh_rate(config_gfx->monitor_check);
    }

    iidxhook_util_d3d9_configure(&d3d9_config);

    hook_d3d9_init(iidxhook_d3d9_handlers, lengthof(iidxhook_d3d9_handlers));
}

/**
 * This seems to be a good entry point to intercept
 * before the game calls anything important
 */
static HANDLE STDCALL
my_OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
    struct cconfig *config;

    struct iidxhook_util_config_eamuse config_eamuse;
    struct iidxhook_config_gfx config_gfx;
    struct iidxhook_config_iidxhook1 config_iidxhook1;
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
    iidxhook_config_iidxhook1_init(config);
    iidxhook_config_misc_init(config);
    iidxhook_config_sec_init(config);

    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK1_INFO_HEADER "\n" IIDXHOOK1_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    iidxhook_util_config_eamuse_get(&config_eamuse, config);
    iidxhook_config_gfx_get(&config_gfx, config);
    iidxhook_config_iidxhook1_get(&config_iidxhook1, config);
    iidxhook_config_misc_get(&config_misc, config);
    iidxhook_config_sec_get(&config_sec, config);

    cconfig_finit(config);

    log_info(IIDXHOOK1_INFO_HEADER);
    log_info("Initializing iidxhook...");

    /* Round plug security */

    ezusb_iidx_emu_node_security_plug_set_boot_version(
        &config_sec.boot_version);
    ezusb_iidx_emu_node_security_plug_set_boot_seeds(config_sec.boot_seeds);
    ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
        &config_sec.black_plug_mcode);
    ezusb_iidx_emu_node_security_plug_set_pcbid(&config_eamuse.pcbid);

    /* Magnetic card reader (9th to HS) */

    ezusb_iidx_emu_node_serial_set_card_attributes(
        0, true, config_eamuse.card_type);

    /* eAmusement server IP */

    eamuse_set_addr(&config_eamuse.server);
    eamuse_check_connection();

    /* Patch rteffect.dll calls */

    if (config_misc.rteffect_stub) {
        effector_hook_init();
    }

    /* Settings paths */

    if (strlen(config_misc.settings_path) > 0) {
        settings_hook_set_path(config_misc.settings_path);
    }

    /* Direct3D and USER32 hooks */

    iidxhook1_setup_d3d9_hooks(&config_gfx, &config_iidxhook1);

    /* Disable operator menu clock setting system clock time */

    if (config_misc.disable_clock_set) {
        iidxhook_util_clock_hook_init();
    }

    /* Start up IIDXIO.DLL */

    log_info("Starting IIDX IO backend");
    iidx_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!iidx_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing IIDX IO backend failed");
    }

    /* Start up EAMIO.DLL */

    log_misc("Initializing card reader backend");
    eam_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!eam_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing card reader backend failed");
    }

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(ezusb_emu_device_dispatch_irp);
    iohook_push_handler(iidxhook_util_chart_patch_dispatch_irp);
    iohook_push_handler(settings_hook_dispatch_irp);

    hook_setupapi_init(&ezusb_emu_desc_device.setupapi);
    ezusb_emu_device_hook_init(ezusb_iidx_emu_msg_init());

    log_info("-------------------------------------------------------------");
    log_info("---------------- End iidxhook my_OpenProcess ----------------");
    log_info("-------------------------------------------------------------");

skip:
    return real_OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
}

/**
 * Hook library for 9th to Happy Sky
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        log_to_writer(log_writer_debug, NULL);

        /* Bootstrap hook for further init tasks (see above) */

        hook_table_apply(
            NULL, "kernel32.dll", init_hook_syms, lengthof(init_hook_syms));

        /* Actual hooks for game specific stuff */

        acp_hook_init();
        adapter_hook_init();
        eamuse_hook_init();
        settings_hook_init();
    }

    return TRUE;
}

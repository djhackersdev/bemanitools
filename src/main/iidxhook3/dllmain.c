#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "ezusb-iidx-emu/node-security-plug.h"
#include "ezusb-iidx-emu/node-serial.h"
#include "ezusb-iidx-emu/nodes.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "ezusb2-iidx-emu/msg.h"

#include "hook/d3d9.h"
#include "hook/iohook.h"
#include "hook/table.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/chart-patch.h"
#include "iidxhook-util/clock.h"
#include "iidxhook-util/config-eamuse.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-misc.h"
#include "iidxhook-util/config-sec.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/eamuse.h"
#include "iidxhook-util/settings.h"

#include "security/rp-sign-key.h"

#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"

#define IIDXHOOK3_INFO_HEADER                           \
    "iidxhook for Gold, DJTroopers, Empress and Sirius" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK3_CMD_USAGE \
    "Usage: inject.exe iidxhook3.dll <bm2dx.exe> [options...]"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static HANDLE STDCALL my_OpenProcess(DWORD, BOOL, DWORD);
static HANDLE(STDCALL *real_OpenProcess)(DWORD, BOOL, DWORD);
static bool iidxhook_init_check;

static const struct hook_symbol init_hook_syms[] = {
    {.name = "OpenProcess",
     .patch = my_OpenProcess,
     .link = (void **) &real_OpenProcess},
};

static void
iidxhook3_setup_d3d9_hooks(const struct iidxhook_config_gfx *config_gfx)
{
    struct iidxhook_util_d3d9_config d3d9_config;

    iidxhook_util_d3d9_init_config(&d3d9_config);

    d3d9_config.windowed = config_gfx->windowed;
    d3d9_config.framed = config_gfx->framed;
    d3d9_config.override_window_width = config_gfx->window_width;
    d3d9_config.override_window_height = config_gfx->window_height;
    d3d9_config.framerate_limit = config_gfx->frame_rate_limit;
    d3d9_config.pci_vid = config_gfx->pci_id_vid;
    d3d9_config.pci_pid = config_gfx->pci_id_pid;

    /* Required for GOLD (and newer?) to not crash with NVIDIA cards */
    d3d9_config.iidx11_to_17_fix_uvs_bg_videos = true;
    d3d9_config.iidx14_to_19_nvidia_fix = true;

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
            IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_14_TO_18_VGA);
    } else if (config_gfx->monitor_check > 0) {
        log_info(
            "Manual monitor check, resulting refresh rate: %f",
            config_gfx->monitor_check);

        iidxhook_util_chart_patch_init(
            IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_14_TO_18_VGA);
        iidxhook_util_chart_patch_set_refresh_rate(config_gfx->monitor_check);
    }

    iidxhook_util_d3d9_configure(&d3d9_config);

    hook_d3d9_init(iidxhook_d3d9_handlers, lengthof(iidxhook_d3d9_handlers));
}

/**
 * This seems to be a good entry point to intercept
 * before the game calls anything important
 */
HANDLE STDCALL
my_OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
    struct cconfig *config;

    struct iidxhook_util_config_eamuse config_eamuse;
    struct iidxhook_config_gfx config_gfx;
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
    iidxhook_config_misc_init(config);
    iidxhook_config_sec_init(config);

    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK3_INFO_HEADER "\n" IIDXHOOK3_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    iidxhook_util_config_eamuse_get(&config_eamuse, config);
    iidxhook_config_gfx_get(&config_gfx, config);
    iidxhook_config_misc_get(&config_misc, config);
    iidxhook_config_sec_get(&config_sec, config);

    cconfig_finit(config);

    log_info(IIDXHOOK3_INFO_HEADER);
    log_info("Initializing iidxhook...");

    /* Round plug security */

    ezusb_iidx_emu_node_security_plug_set_boot_version(
        &config_sec.boot_version);
    ezusb_iidx_emu_node_security_plug_set_boot_seeds(config_sec.boot_seeds);
    ezusb_iidx_emu_node_security_plug_set_plug_black_sign_key(
        &security_rp_sign_key_black_iidx);
    ezusb_iidx_emu_node_security_plug_set_plug_white_sign_key(
        &security_rp_sign_key_white_eamuse);
    ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
        &config_sec.black_plug_mcode);
    ezusb_iidx_emu_node_security_plug_set_plug_white_mcode(
        &security_mcode_eamuse);
    ezusb_iidx_emu_node_security_plug_set_pcbid(&config_eamuse.pcbid);
    ezusb_iidx_emu_node_security_plug_set_eamid(&config_eamuse.eamid);

    /* eAmusement server IP */

    eamuse_set_addr(&config_eamuse.server);
    eamuse_check_connection();

    /* Direct3D and USER32 hooks */

    iidxhook3_setup_d3d9_hooks(&config_gfx);

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
    iohook_push_handler(ezusb2_emu_device_dispatch_irp);
    iohook_push_handler(iidxhook_util_acio_dispatch_irp);
    iohook_push_handler(iidxhook_util_chart_patch_dispatch_irp);
    iohook_push_handler(settings_hook_dispatch_irp);

    hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
    ezusb2_emu_device_hook_init(ezusb2_iidx_emu_msg_init());

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
 * Hook library for Gold to Sirius
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        log_to_writer(log_writer_debug, NULL);

        /* Bootstrap hook for further init tasks (see above) */

        hook_table_apply(
            NULL, "kernel32.dll", init_hook_syms, lengthof(init_hook_syms));

        /* Actual hooks for game specific stuff */
        /* Some hooks are setting dependent and can only be applied later in
           the bootstrap hook */

        acp_hook_init();
        adapter_hook_init();
        eamuse_hook_init();
        settings_hook_init();
    }

    return TRUE;
}

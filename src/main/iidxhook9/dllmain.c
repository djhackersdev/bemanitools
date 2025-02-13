#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/config-adapter.h"
#include "hooklib/memfile.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/log-server.h"

#include "bio2emu/emu.h"

#include "bio2emu-iidx/bi2a.h"

#include "iidxhook9/config-io.h"
#include "iidxhook9/fs-hook.h"

#include "camhook/cam.h"
#include "camhook/config-cam.h"

#include "asio/asio-reghook.h"
#include "asio/config-asio.h"
#include "d3d9exhook/d3d9ex.h"
#include "dinput/dinput.h"

#include "imports/avs.h"

#include "util/cmdline.h"
#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"

#define IIDXHOOK9_INFO_HEADER   \
    "iidxhook for Heroic Verse" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
#define IIDXHOOK9_CMD_USAGE                                          \
    "Usage: launcher.exe -B iidxhook9-prehook.dll -K iidxhook9.dll " \
    "<bm2dx.dll> [options...]"

static struct iidxhook9_config_io iidxhook9_config_io;
static struct camhook_config_cam config_cam;
static struct asiohook_config_asio config_asio;
static struct hooklib_config_adapter config_adapter;
static struct d3d9exhook_config_gfx config_gfx;

static struct bio2emu_port bio2_emu = {
    .port = "COM4",
    .wport = L"COM4",
    .dispatcher = bio2_emu_bi2a_dispatch_request,
};

static bool load_configs()
{
    struct cconfig *config;
    config = cconfig_init();

    iidxhook9_config_io_init(config);
    camhook_config_cam_init(config, 2, true);

    d3d9exhook_config_gfx_init(config);

    hooklib_config_adapter_init(config);
    asiohook_config_init(config);

    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK9_INFO_HEADER "\n" IIDXHOOK9_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        return false;
    }

    iidxhook9_config_io_get(&iidxhook9_config_io, config);
    camhook_config_cam_get(&config_cam, config, 2, true);

    d3d9exhook_config_gfx_get(&config_gfx, config);

    hooklib_config_adapter_get(&config_adapter, config);
    asiohook_config_asio_get(&config_asio, config);

    cconfig_finit(config);

    return true;
}

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    // log_server_init is required due to IO occuring in a non avs_thread
    log_server_init();

    log_info("-------------------------------------------------------------");
    log_info("--------------- Begin iidxhook dll_entry_init ---------------");
    log_info("-------------------------------------------------------------");

    log_info(IIDXHOOK9_INFO_HEADER);
    log_info("Initializing iidxhook...");

    // reload configs again so they get logged through avs as well
    // (so we get a copy of them in the -Y logfile)
    if (!load_configs()) {
        log_server_fini();
        exit(EXIT_FAILURE);
    }

    d3d9ex_configure(&config_gfx);
    d3d9ex_hook_init();

    acp_hook_init();
    adapter_hook_init();
    dinput_init();

    /* Start up IIDXIO.DLL */
    if (!iidxhook9_config_io.disable_bio2_emu) {
        log_info("Starting IIDX IO backend");
        iidx_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        if (!iidx_io_init(
                avs_thread_create, avs_thread_join, avs_thread_destroy)) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    }

    /* Start up EAMIO.DLL */
    if (!iidxhook9_config_io.disable_card_reader_emu) {
        log_misc("Initializing card reader backend");
        eam_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        if (!eam_io_init(
                avs_thread_create, avs_thread_join, avs_thread_destroy)) {
            log_fatal("Initializing card reader backend failed");
        }
    }

    iohook_push_handler(iidxhook_util_acio_dispatch_irp);
    iohook_push_handler(bio2emu_port_dispatch_irp);

    if (!iidxhook9_config_io.disable_file_hooks) {
        memfile_hook_init();
        iohook_push_handler(memfile_hook_dispatch_irp);

        // game uses this file to determine what mode to put the cab in
        // the default depends on a value embedded in the dll
        if (iidxhook9_config_io.lightning_mode) {
            memfile_hook_add_fd("d:\\\\001rom.txt", ABSOLUTE_MATCH, "TDJ", 3);
        } else {
            memfile_hook_add_fd("d:\\\\001rom.txt", ABSOLUTE_MATCH, "LDJ", 3);
        }

        // redirect F:\ drive to vfs (used for video recording)
        iidxhook9_fs_hooks_init();
    }

    rs232_hook_init();
    rs232_hook_limit_hooks();

    if (!iidxhook9_config_io.disable_bio2_emu) {
        if (!iidxhook9_config_io.lightning_mode) {
            bio2emu_init();
            bio2_emu_bi2a_set_tt_multiplier(iidxhook9_config_io.tt_multiplier);
            bio2_emu_bi2a_init(
                &bio2_emu, iidxhook9_config_io.disable_poll_limiter);
        }
    }

    if (!iidxhook9_config_io.disable_card_reader_emu) {
        if (iidxhook9_config_io.lightning_mode) {
            // TDJ mode expects 1.7.0 readers
            iidxhook_util_acio_override_version(v170);
        }

        iidxhook_util_acio_init(false);
    }

    // camera hooks
    if (!config_cam.disable_emu) {
        camhook_set_version(CAMHOOK_VERSION_NEW);
        camhook_init(&config_cam);
    }

    adapter_hook_override(config_adapter.override_ip);

    // asio hooks
    if (config_asio.force_asio) {
        asio_reghook_init("XONAR SOUND CARD(64)", config_asio.replacement_name);
    }

    log_info("-------------------------------------------------------------");
    log_info("---------------- End iidxhook dll_entry_init ----------------");
    log_info("-------------------------------------------------------------");

    return app_hook_invoke_init(sidcode, param);
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    if (!config_cam.disable_emu) {
        camhook_fini();
    }

    if (!iidxhook9_config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");
        eam_io_fini();
    }

    if (!iidxhook9_config_io.disable_bio2_emu) {
        log_misc("Shutting down IIDX IO backend");
        iidx_io_fini();
    }

    if (!iidxhook9_config_io.disable_file_hooks) {
        memfile_hook_fini();
    }

    log_server_fini();

    return result;
}

static void pre_hook()
{
    log_info("-------------------------------------------------------------");
    log_info("------------------ Begin iidxhook pre_hook ------------------");
    log_info("-------------------------------------------------------------");

    if (!load_configs()) {
        exit(EXIT_FAILURE);
    }

    // asio hooks
    if (config_asio.force_asio) {
        SetEnvironmentVariable("SOUND_OUTPUT_DEVICE", "asio");
    } else if (config_asio.force_wasapi) {
        SetEnvironmentVariable("SOUND_OUTPUT_DEVICE", "wasapi");
    }

    if (iidxhook9_config_io.disable_cams) {
        // this disables the entire camera subsystem
        // useful for skipping the camera error entierly
        SetEnvironmentVariable("CONNECT_CAMERA", "0");
    }

    log_info("-------------------------------------------------------------");
    log_info("------------------- End iidxhook pre_hook -------------------");
    log_info("-------------------------------------------------------------");
}

/**
 * Hook library HV+
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    if (avs_is_active()) {
        // if AVS is loaded, we're likely too late to be a prehook
        // so we warn the user
        // and switch the current logging context to AVS so it shows up in logs
        log_to_external(
            log_body_misc, log_body_info, log_body_warning, log_body_fatal);
        log_warning("iidxhook9 is designed to be used as a prehook");
        log_warning("please ensure that it is being loaded with -B");
        log_fatal("cya l8r in the prehook :3");
    } else {
        // we can't log to external in DllMain (AVS) as we're a prehook
        // later during my_dll_entry_init, log_server_init is called
        // which sets swaps the main log write to that instead
        log_to_writer(log_writer_file, stdout);
    }

    pre_hook();

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

end:
    return TRUE;
}

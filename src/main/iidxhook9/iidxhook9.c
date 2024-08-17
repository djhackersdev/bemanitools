#define LOG_MODULE "iidxhook9"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/config-adapter.h"
#include "hooklib/memfile.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/iidx.h"

#include "iidxhook-util/acio.h"

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

#include "module/io-ext.h"
#include "module/io.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/cmdline.h"
#include "util/str.h"

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

static module_io_t *iidxhook_module_io_iidx;
static module_io_t *iidxhook_module_io_eam;

static void _iidxhook9_io_iidx_init(module_io_t **module)
{
    bt_io_iidx_api_t api;

    module_io_ext_load_and_init(
        "iidxio.dll", "bt_module_io_iidx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_iidx_api_set(&api);
}

static void _iidxhook9_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static void load_configs(const bt_core_config_t *config)
{
    iidxhook9_config_io_get(config, &iidxhook9_config_io);

    camhook_config_cam_get(config, &config_cam, 2, true);

    d3d9exhook_config_gfx_get(config, &config_gfx);

    hooklib_config_adapter_get(config, &config_adapter);
    asiohook_config_asio_get(config, &config_asio);
}

static bool _iidxhook9_pre_avs_init(const bt_core_config_t *config)
{
    load_configs(config);

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

    return true;
}

static bool
_iidxhook9_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    log_info("iidxhook for Heroic Verse, Bistrover, Cast Hour and Resident");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    // reload configs again so they get logged through avs as well
    // (so we get a copy of them in the -Y logfile)
    load_configs(config);

    d3d9ex_configure(&config_gfx);
    d3d9ex_hook_init();

    acp_hook_init();
    adapter_hook_init();
    dinput_init();

    /* Start up IIDXIO.DLL */
    if (!iidxhook9_config_io.disable_bio2_emu) {
        log_info("Starting IIDX IO backend");

        _iidxhook9_io_iidx_init(&iidxhook_module_io_iidx);

        if (!bt_io_iidx_init()) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    }

    /* Start up EAMIO.DLL */
    if (!iidxhook9_config_io.disable_card_reader_emu) {
        log_misc("Initializing card reader backend");

        _iidxhook9_io_eam_init(&iidxhook_module_io_eam);

        if (!bt_io_eam_init()) {
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

    return true;
}

static void _iidxhook9_main_fini()
{
    if (!config_cam.disable_emu) {
        camhook_fini();
    }

    if (!iidxhook9_config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");
        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&iidxhook_module_io_eam);
    }

    if (!iidxhook9_config_io.disable_bio2_emu) {
        log_misc("Shutting down IIDX IO backend");
        bt_io_iidx_fini();

        bt_io_iidx_api_clear();
        module_io_free(&iidxhook_module_io_iidx);
    }

    if (!iidxhook9_config_io.disable_file_hooks) {
        memfile_hook_fini();
    }
}

void bt_module_core_config_api_set(const bt_core_config_api_t *api)
{
    bt_core_config_api_set(api);
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

    api->v1.pre_avs_init = _iidxhook9_pre_avs_init;
    api->v1.main_init = _iidxhook9_main_init;
    api->v1.main_fini = _iidxhook9_main_fini;
}
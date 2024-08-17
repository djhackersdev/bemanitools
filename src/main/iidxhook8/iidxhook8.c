#define LOG_MODULE "iidxhook8"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hook/d3d9.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/iidx.h"

#include "iidxhook-d3d9/bb-scale-hd.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/d3d9.h"

#include "bio2emu-iidx/bi2a.h"
#include "bio2emu/emu.h"

#include "camhook/cam.h"
#include "camhook/config-cam.h"
#include "iidxhook8/config-io.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/str.h"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_d3d9_bb_scale_hd_d3d9_irp_handler,
    iidxhook_util_d3d9_irp_handler,
};

static module_io_t *iidxhook_module_io_iidx;
static module_io_t *iidxhook_module_io_eam;

static void _iidxhook8_io_iidx_init(module_io_t **module)
{
    bt_io_iidx_api_t api;

    module_io_ext_load_and_init(
        "iidxio.dll", "bt_module_io_iidx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_iidx_api_set(&api);
}

static void _iidxhook8_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static void
iidxhook8_setup_d3d9_hooks(const struct iidxhook_config_gfx *config_gfx)
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
    d3d9_config.forced_refresh_rate = config_gfx->forced_refresh_rate;
    d3d9_config.device_adapter = config_gfx->device_adapter;

    iidxhook_util_d3d9_configure(&d3d9_config);

    // The "old"/current scaling feature does not work with 20-26 because
    // the render engine changed and provides its own built-in scaling feature
    if (config_gfx->scale_back_buffer_width > 0 &&
        config_gfx->scale_back_buffer_height > 0) {
        iidxhook_d3d9_bb_scale_hd_init(
            config_gfx->scale_back_buffer_width,
            config_gfx->scale_back_buffer_height);
    }

    hook_d3d9_init(iidxhook_d3d9_handlers, lengthof(iidxhook_d3d9_handlers));
}

struct iidxhook8_config_io iidxhook8_config_io;
struct camhook_config_cam config_cam;

static struct bio2emu_port bio2_emu = {
    .port = "COM4",
    .wport = L"COM4",
    .dispatcher = bio2_emu_bi2a_dispatch_request,
};

static bool
_iidxhook8_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    iidxhook_config_gfx_t config_gfx;

    log_info("iidxhook for Cannon Ballers and Rootage");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    iidxhook_util_config_gfx_get(config, &config_gfx);
    iidxhook8_config_io_get(config, &iidxhook8_config_io);
    camhook_config_cam_get(config, &config_cam, 2, false);

    acp_hook_init();
    adapter_hook_init();

    iidxhook8_setup_d3d9_hooks(&config_gfx);

    /* Start up IIDXIO.DLL */
    if (!iidxhook8_config_io.disable_bio2_emu) {
        log_info("Starting IIDX IO backend");

        _iidxhook8_io_iidx_init(&iidxhook_module_io_iidx);

        if (!bt_io_iidx_init()) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    }

    /* Start up EAMIO.DLL */
    if (!iidxhook8_config_io.disable_card_reader_emu) {
        log_misc("Initializing card reader backend");

        _iidxhook8_io_eam_init(&iidxhook_module_io_eam);

        if (!bt_io_eam_init()) {
            log_fatal("Initializing card reader backend failed");
        }
    }

    /* iohooks are okay, even if emu is diabled since the fake handlers won't be
     * used */
    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(iidxhook_util_acio_dispatch_irp);
    iohook_push_handler(bio2emu_port_dispatch_irp);

    rs232_hook_init();
    rs232_hook_limit_hooks();

    if (!iidxhook8_config_io.disable_bio2_emu) {
        bio2emu_init();
        bio2_emu_bi2a_init(&bio2_emu, iidxhook8_config_io.disable_poll_limiter);
    }

    if (!iidxhook8_config_io.disable_card_reader_emu) {
        iidxhook_util_acio_init(false);
    }

    // camera hooks
    if (!config_cam.disable_emu) {
        camhook_init(&config_cam);
    }

    return true;
}

static void _iidxhook8_main_fini()
{
    if (!config_cam.disable_emu) {
        camhook_fini();
    }

    if (!iidxhook8_config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");
        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&iidxhook_module_io_eam);
    }

    if (!iidxhook8_config_io.disable_bio2_emu) {
        log_misc("Shutting down IIDX IO backend");
        bt_io_iidx_fini();

        bt_io_iidx_api_clear();
        module_io_free(&iidxhook_module_io_iidx);
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

    api->v1.main_init = _iidxhook8_main_init;
    api->v1.main_fini = _iidxhook8_main_fini;
}

#define LOG_MODULE "iidxhook3"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ezusb-emu/node-security-plug.h"
#include "ezusb-iidx-emu/nodes.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "ezusb2-iidx-emu/msg.h"

#include "hook/d3d9.h"
#include "hook/iohook.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/iidx.h"

#include "iidxhook3/avs-boot.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/chart-patch.h"
#include "iidxhook-util/clock.h"
#include "iidxhook-util/config-eamuse.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-misc.h"
#include "iidxhook-util/config-sec.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/settings.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "security/rp-sign-key.h"

#include "util/str.h"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static module_io_t *_iidxhook3_module_io_iidx;
static module_io_t *_iidxhook3_module_io_eam;

static void _iidxhook3_io_iidx_init(module_io_t **module)
{
    bt_io_iidx_api_t api;

    module_io_ext_load_and_init(
        "iidxio.dll", "bt_module_io_iidx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_iidx_api_set(&api);
}

static void _iidxhook3_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static void iidxhook3_setup_d3d9_hooks(
    const struct iidxhook_config_gfx *config_gfx)
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
    d3d9_config.iidx09_to_17_fix_uvs_bg_videos = true;
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
            IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_14_TO_19_VGA);
    } else if (config_gfx->monitor_check > 0) {
        log_info(
            "Manual monitor check, resulting refresh rate: %f",
            config_gfx->monitor_check);

        iidxhook_util_chart_patch_init(
            IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_14_TO_19_VGA);
        iidxhook_util_chart_patch_set_refresh_rate(config_gfx->monitor_check);
    }

    iidxhook_util_d3d9_configure(&d3d9_config);

    hook_d3d9_init(iidxhook_d3d9_handlers, lengthof(iidxhook_d3d9_handlers));
}

static bool
_iidxhook3_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    iidxhook_util_config_eamuse_t config_eamuse;
    iidxhook_config_gfx_t config_gfx;
    iidxhook_config_misc_t config_misc;
    iidxhook_config_sec_t config_sec;

    log_info("iidxhook for Gold, DJTroopers, Empress and Sirius");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    iidxhook_util_config_eamuse_get(config, &config_eamuse);
    iidxhook_util_config_gfx_get(config, &config_gfx);
    iidxhook_util_config_misc_get(config, &config_misc);
    iidxhook_util_config_sec_get(config, &config_sec);

    acp_hook_init();
    adapter_hook_init();
    settings_hook_init();

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

    iidxhook3_avs_boot_init();
    iidxhook3_avs_boot_set_eamuse_addr(&config_eamuse.server);

    /* Settings paths */

    if (strlen(config_misc.settings_path) > 0) {
        settings_hook_set_path(config_misc.settings_path);
    }

    /* Direct3D and USER32 hooks */

    iidxhook3_setup_d3d9_hooks(&config_gfx);

    /* Disable operator menu clock setting system clock time */

    if (config_misc.disable_clock_set) {
        iidxhook_util_clock_hook_init();
    }

    /* Start up IIDXIO.DLL */

    log_info("Starting IIDX IO backend");

    _iidxhook3_io_iidx_init(&_iidxhook3_module_io_iidx);

    if (!bt_io_iidx_init()) {
        log_fatal("Initializing IIDX IO backend failed");
    }

    /* Start up EAMIO.DLL */

    log_misc("Initializing card reader backend");

    _iidxhook3_io_eam_init(&_iidxhook3_module_io_eam);

    if (!bt_io_eam_init()) {
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

    return true;
}

static void _iidxhook3_main_fini()
{
    // TODO cleanup
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

    api->v1.main_init = _iidxhook3_main_init;
    api->v1.main_fini = _iidxhook3_main_fini;
}
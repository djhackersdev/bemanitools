#define LOG_MODULE "iidxhook1"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ezusb-emu/desc.h"
#include "ezusb-emu/device.h"
#include "ezusb-emu/node-security-plug.h"

#include "ezusb-iidx-emu/msg.h"
#include "ezusb-iidx-emu/node-serial.h"
#include "ezusb-iidx-emu/nodes.h"

#include "hook/d3d9.h"
#include "hook/iohook.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/setupapi.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/iidx.h"

#include "iidxhook-util/chart-patch.h"
#include "iidxhook-util/clock.h"
#include "iidxhook-util/config-eamuse.h"
#include "iidxhook-util/config-ezusb.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-misc.h"
#include "iidxhook-util/config-sec.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/eamuse.h"
#include "iidxhook-util/effector.h"
#include "iidxhook-util/settings.h"

#include "iidxhook1/ezusb-mon.h"
#include "iidxhook1/log-ezusb.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/proc.h"

static const hook_d3d9_irp_handler_t _iidxhook1_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static module_io_t *_iidxhook1_module_io_iidx;
static module_io_t *_iidxhook1_module_io_eam;

static void _iidxhook1_io_iidx_init(module_io_t **module)
{
    bt_io_iidx_api_t api;

    module_io_ext_load_and_init(
        "iidxio.dll", "bt_module_io_iidx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_iidx_api_set(&api);
}

static void _iidxhook1_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static void iidxhook1_setup_d3d9_hooks(
    const struct iidxhook_config_gfx *config_gfx)
{
    struct iidxhook_util_d3d9_config d3d9_config;

    if (!proc_is_module_loaded("d3d9.dll")) {
        log_fatal(
            "d3d8 hook module deprecated, using d3d9 hook modules requires "
            "d3d8to9 to work! Could not detect loaded d3d9.dll");
    }

    iidxhook_util_d3d9_init_config(&d3d9_config);

    d3d9_config.windowed = config_gfx->windowed;
    d3d9_config.framed = config_gfx->framed;
    d3d9_config.override_window_width = config_gfx->window_width;
    d3d9_config.override_window_height = config_gfx->window_height;
    d3d9_config.framerate_limit = config_gfx->frame_rate_limit;
    d3d9_config.iidx12_fix_song_select_bg =
        config_gfx->happy_sky_ms_bg_fix;
    d3d9_config.iidx09_to_17_fix_uvs_bg_videos = config_gfx->bgvideo_uv_fix;
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

    hook_d3d9_init(_iidxhook1_d3d9_handlers, lengthof(_iidxhook1_d3d9_handlers));
}

static bool
_iidxhook1_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    iidxhook_util_config_ezusb_t config_ezusb;
    iidxhook_util_config_eamuse_t config_eamuse;
    iidxhook_config_gfx_t config_gfx;
    iidxhook_config_misc_t config_misc;
    iidxhook_config_sec_t config_sec;

    log_info("iidxhook for 9th Style, 10th Style, RED and HAPPY SKY");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    iidxhook_util_config_ezusb_get2(config, &config_ezusb);
    iidxhook_util_config_eamuse_get2(config, &config_eamuse);
    iidxhook_util_config_gfx_get2(config, &config_gfx);
    iidxhook_util_config_misc_get2(config, &config_misc);
    iidxhook_util_config_sec_get2(config, &config_sec);

    acp_hook_init();
    adapter_hook_init();
    eamuse_hook_init();
    settings_hook_init();

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

    // TODO this doesn't work, because it's too late to apply this here
    // this needs to be launched before bm2dx.exe even loads the rteffect.dll
    // and runs DllMain of it (see also how this is done with the proxy
    // ezusb.dll)
    if (config_misc.rteffect_stub) {
        effector_hook_init();
    }

    /* Settings paths */

    if (strlen(config_misc.settings_path) > 0) {
        settings_hook_set_path(config_misc.settings_path);
    }

    /* Direct3D and USER32 hooks */

    iidxhook1_setup_d3d9_hooks(&config_gfx);

    /* Disable operator menu clock setting system clock time */

    if (config_misc.disable_clock_set) {
        iidxhook_util_clock_hook_init();
    }

    /* Start up IIDXIO.DLL */

    log_info("Starting IIDX IO backend");

    _iidxhook1_io_iidx_init(&_iidxhook1_module_io_iidx);

    if (!bt_io_iidx_init()) {
        log_fatal("Initializing IIDX IO backend failed");
    }

    /* Start up EAMIO.DLL */

    log_misc("Initializing card reader backend");

    _iidxhook1_io_eam_init(&_iidxhook1_module_io_eam);

    if (!bt_io_eam_init()) {
        log_fatal("Initializing card reader backend failed");
    }

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(ezusb_emu_device_dispatch_irp);
    iohook_push_handler(iidxhook_util_chart_patch_dispatch_irp);
    iohook_push_handler(settings_hook_dispatch_irp);

    hook_setupapi_init(&ezusb_emu_desc_device.setupapi);
    ezusb_emu_device_hook_init(
        ezusb_iidx_emu_msg_init(config_ezusb.io_board_type));

    if (config_ezusb.api_call_monitoring) {
        ezusb_log_hook_init();
        ezusb_mon_hook_init();
    }

    return true;
}
static void _iidxhook1_main_fini()
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

    api->v1.main_init = _iidxhook1_main_init;
    api->v1.main_fini = _iidxhook1_main_fini;
}

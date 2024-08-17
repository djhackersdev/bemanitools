#define LOG_MODULE "iidxhook4-cn"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ezusb-emu/node-security-plug.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "ezusb2-iidx-emu/msg.h"

#include "hook/table.h"

#include "hooklib/acp.h"
#include "hooklib/setupapi.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/iidx.h"

#include "iidxhook4-cn/avs-boot.h"
#include "iidxhook4-cn/path.h"

#include "iidxhook-util/chart-patch.h"
#include "iidxhook-util/config-eamuse.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-io.h"
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

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static module_io_t *iidxhook_module_io_iidx;

static void _iidxhook4_cn_io_iidx_init(module_io_t **module)
{
    bt_io_iidx_api_t api;

    module_io_ext_load_and_init(
        "iidxio.dll", "bt_module_io_iidx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_iidx_api_set(&api);
}

static void
iidxhook4_cn_setup_d3d9_hooks(const struct iidxhook_config_gfx *config_gfx)
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
    d3d9_config.scale_back_buffer_width = config_gfx->scale_back_buffer_width;
    d3d9_config.scale_back_buffer_height = config_gfx->scale_back_buffer_height;
    d3d9_config.scale_back_buffer_filter = config_gfx->scale_back_buffer_filter;
    d3d9_config.forced_refresh_rate = config_gfx->forced_refresh_rate;
    d3d9_config.device_adapter = config_gfx->device_adapter;
    d3d9_config.iidx18_and_19_diagonal_tearing_fix =
        config_gfx->diagonal_tearing_fix;
    d3d9_config.iidx14_to_19_nvidia_fix = true;
    d3d9_config.iidx18_and_19_diagonal_tearing_fix =
        config_gfx->diagonal_tearing_fix;

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
_iidxhook4_cn_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    iidxhook_util_config_eamuse_t config_eamuse;
    iidxhook_config_gfx_t config_gfx;
    iidxhook_config_sec_t config_sec;
    iidxhook_config_misc_t config_misc;
    iidxhook_config_io_t config_io;

    log_info("iidxhook for Resort Anthem CN");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    iidxhook_util_config_eamuse_get(config, &config_eamuse);
    iidxhook_util_config_gfx_get(config, &config_gfx);
    iidxhook_util_config_sec_get(config, &config_sec);
    iidxhook_util_config_misc_get(config, &config_misc);
    iidxhook_util_config_io_get(config, &config_io);

    iidxhook4_cn_path_init();
    iidxhook4_cn_avs_boot_init();
    acp_hook_init();
    settings_hook_init();

    /**
     * This game is using a black round plug for game license management instead
     * of a black usb dongle. No white dongle hooks applies since the game does
     * not have network functionality. Also, card readers are not used/checked;
     * no card reader hooks required.
     */
    ezusb_iidx_emu_node_security_plug_set_boot_version(
        &config_sec.boot_version);
    ezusb_iidx_emu_node_security_plug_set_boot_seeds(config_sec.boot_seeds);
    ezusb_iidx_emu_node_security_plug_set_plug_black_sign_key(
        &security_rp_sign_key_black_iidx);
    ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
        &config_sec.black_plug_mcode);
    ezusb_iidx_emu_node_security_plug_set_pcbid(&config_eamuse.pcbid);

    iidxhook4_cn_setup_d3d9_hooks(&config_gfx);

    if (strlen(config_misc.settings_path) > 0) {
        settings_hook_set_path(config_misc.settings_path);
    }

    if (!config_io.disable_io_emu) {
        log_info("Starting IIDX IO backend");

        _iidxhook4_cn_io_iidx_init(&iidxhook_module_io_iidx);

        if (!bt_io_iidx_init()) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    } else {
        log_info("IIDX IO emulation backend disabled");
    }

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(ezusb2_emu_device_dispatch_irp);
    iohook_push_handler(iidxhook_util_chart_patch_dispatch_irp);
    iohook_push_handler(settings_hook_dispatch_irp);

    if (!config_io.disable_io_emu) {
        hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
        ezusb2_emu_device_hook_init(ezusb2_iidx_emu_msg_init());
    }

    return true;
}

static void _iidxhook4_cn_main_fini()
{
    // TODO cleanup initialized stuff
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

    api->v1.main_init = _iidxhook4_cn_main_init;
    api->v1.main_fini = _iidxhook4_cn_main_fini;
}

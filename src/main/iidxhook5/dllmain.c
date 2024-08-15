#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "avs-ext/log.h"
#include "avs-ext/thread.h"

#include "cconfig/cconfig-hook.h"

#include "core/boot.h"

#include "ezusb-iidx-emu/nodes.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "ezusb2-iidx-emu/msg.h"

#include "hook/d3d9.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/iidx.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/chart-patch.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-io.h"
#include "iidxhook-util/config-misc.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/log-server.h"
#include "iidxhook-util/settings.h"

#include "imports/avs.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "util/str.h"

#include "ifs-snd-redir.h"

#define IIDXHOOK5_INFO_HEADER \
    "iidxhook for Lincle"     \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK5_CMD_USAGE \
    "Usage: launcher.exe -K iidxhook5.dll <bm2dx.dll> [options...]"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static struct iidxhook_config_io config_io;
static module_io_t *iidxhook_module_io_iidx;
static module_io_t *iidxhook_module_io_eam;

static void _iidxhook5_io_iidx_init(module_io_t **module)
{
    bt_io_iidx_api_t api;

    module_io_ext_load_and_init(
        "iidxio.dll", "bt_module_io_iidx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_iidx_api_set(&api);
}

static void _iidxhook5_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static void
iidxhook5_setup_d3d9_hooks(const struct iidxhook_config_gfx *config_gfx)
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

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    struct cconfig *config;

    struct iidxhook_config_gfx config_gfx;
    struct iidxhook_config_misc config_misc;

    log_server_init();
    log_info("-------------------------------------------------------------");
    log_info("--------------- Begin iidxhook dll_entry_init ---------------");
    log_info("-------------------------------------------------------------");

    config = cconfig_init();

    iidxhook_config_gfx_init(config);
    iidxhook_config_io_init(config);
    iidxhook_config_misc_init(config);

    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK5_INFO_HEADER "\n" IIDXHOOK5_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        log_server_fini();
        exit(EXIT_FAILURE);
    }

    iidxhook_config_gfx_get(&config_gfx, config);
    iidxhook_config_io_get(&config_io, config);
    iidxhook_config_misc_get(&config_misc, config);

    cconfig_finit(config);

    log_info(IIDXHOOK5_INFO_HEADER);
    log_info("Initializing iidxhook...");

    iidxhook5_setup_d3d9_hooks(&config_gfx);

    if (strlen(config_misc.settings_path) > 0) {
        settings_hook_set_path(config_misc.settings_path);
    }

    if (!config_io.disable_io_emu) {
        log_info("Starting IIDX IO backend");

        _iidxhook5_io_iidx_init(&iidxhook_module_io_iidx);

        if (!bt_io_iidx_init()) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    } else {
        log_info("IIDX IO emulation backend disabled");
    }

    if (!config_io.disable_card_reader_emu) {
        log_misc("Initializing card reader backend");

        _iidxhook5_io_eam_init(&iidxhook_module_io_eam);

        if (!bt_io_eam_init()) {
            log_fatal("Initializing card reader backend failed");
        }
    } else {
        log_info("Card reader emulation backend disabled");
    }

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(ezusb2_emu_device_dispatch_irp);
    iohook_push_handler(iidxhook_util_acio_dispatch_irp);
    iohook_push_handler(iidxhook_util_chart_patch_dispatch_irp);
    iohook_push_handler(settings_hook_dispatch_irp);

    if (!config_io.disable_io_emu) {
        hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
        ezusb2_emu_device_hook_init(ezusb2_iidx_emu_msg_init());
    }

    iidxhook5_ifs_snd_redir_init();

    /* Card reader emulation, same issue with hooking as IO emulation */
    rs232_hook_init();

    if (!config_io.disable_card_reader_emu) {
        /* Do not use legacy mode, first version with wave pass readers */
        iidxhook_util_acio_init(false);
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

    if (!config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");
        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&iidxhook_module_io_eam);
    }

    if (!config_io.disable_io_emu) {
        log_misc("Shutting down IIDX IO backend");
        bt_io_iidx_fini();

        bt_io_iidx_api_clear();
        module_io_free(&iidxhook_module_io_iidx);
    }

    log_server_fini();

    return result;
}

/**
 * Hook library for Lincle
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    core_boot("iidxhook5");

    // Use AVS APIs
    avs_ext_log_core_api_set();
    avs_ext_thread_core_api_set();

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    acp_hook_init();
    adapter_hook_init();
    settings_hook_init();

end:
    return TRUE;
}

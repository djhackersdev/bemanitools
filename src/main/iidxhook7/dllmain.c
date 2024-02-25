#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "avs-util/core-interop.h"

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "core/log.h"
#include "core/thread.h"

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

#include "iidxhook-d3d9/bb-scale-hd.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-io.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/log-server.h"

#include "imports/avs.h"

#include "util/str.h"

#define IIDXHOOK7_INFO_HEADER                         \
    "iidxhook for SPADA, PENDUAL, copula and SINOBUZ" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK7_CMD_USAGE \
    "Usage: launcher.exe -K iidxhook7.dll <bm2dx.dll> [options...]"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_d3d9_bb_scale_hd_d3d9_irp_handler,
    iidxhook_util_d3d9_irp_handler,
};

static struct iidxhook_config_io config_io;

static void
iidxhook7_setup_d3d9_hooks(const struct iidxhook_config_gfx *config_gfx)
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

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    struct cconfig *config;

    struct iidxhook_config_gfx config_gfx;

    log_server_init();
    log_info("-------------------------------------------------------------");
    log_info("--------------- Begin iidxhook dll_entry_init ---------------");
    log_info("-------------------------------------------------------------");

    config = cconfig_init();

    iidxhook_config_gfx_init(config);
    iidxhook_config_io_init(config);

    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK7_INFO_HEADER "\n" IIDXHOOK7_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        log_server_fini();
        exit(EXIT_FAILURE);
    }

    iidxhook_config_gfx_get(&config_gfx, config);
    iidxhook_config_io_get(&config_io, config);

    cconfig_finit(config);

    log_info(IIDXHOOK7_INFO_HEADER);
    log_info("Initializing iidxhook...");

    iidxhook7_setup_d3d9_hooks(&config_gfx);

    if (!config_io.disable_io_emu) {
        log_info("Starting IIDX IO backend");

        core_log_impl_assign(iidx_io_set_loggers);

        if (!iidx_io_init(
                core_thread_create_impl_get(),
                core_thread_join_impl_get(),
                core_thread_destroy_impl_get())) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    } else {
        log_info("IIDX IO emulation backend disabled");
    }

    if (!config_io.disable_card_reader_emu) {
        log_misc("Initializing card reader backend");

        core_log_impl_assign(eam_io_set_loggers);

        if (!eam_io_init(
                core_thread_create_impl_get(),
                core_thread_join_impl_get(),
                core_thread_destroy_impl_get())) {
            log_fatal("Initializing card reader backend failed");
        }
    } else {
        log_info("Card reader emulation backend disabled");
    }

    /* iohooks are okay, even if emu is diabled since the fake handlers won't be
     * used */
    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(ezusb2_emu_device_dispatch_irp);
    iohook_push_handler(iidxhook_util_acio_dispatch_irp);

    if (!config_io.disable_io_emu) {
        hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
        ezusb2_emu_device_hook_init(ezusb2_iidx_emu_msg_init());
    }

    /* Card reader emulation, same issue with hooking as IO emulation */
    rs232_hook_init();

    if (!config_io.disable_card_reader_emu) {
        /* Do not use legacy mode, wave pass readers */
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
        eam_io_fini();
    }

    if (!config_io.disable_io_emu) {
        log_misc("Shutting down IIDX IO backend");
        iidx_io_fini();
    }

    log_server_fini();

    return result;
}

/**
 * Hook library for SPADA, PENDUAL, copula and SINOBUZ
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    // Use AVS APIs
    avs_util_core_interop_thread_avs_impl_set();
    avs_util_core_interop_log_avs_impl_set();

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    acp_hook_init();
    adapter_hook_init();

end:
    return TRUE;
}

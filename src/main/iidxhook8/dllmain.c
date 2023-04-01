#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "hook/d3d9.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/log-server.h"

#include "bio2emu-iidx/bi2a.h"
#include "bio2emu/emu.h"

#include "camhook/cam.h"
#include "camhook/config-cam.h"
#include "iidxhook8/config-io.h"

#include "imports/avs.h"

#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"

#define IIDXHOOK8_INFO_HEADER             \
    "iidxhook for Cannon Ballers/Rootage" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
#define IIDXHOOK8_CMD_USAGE \
    "Usage: launcher.exe -K iidxhook8.dll <bm2dx.dll> [options...]"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

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
    d3d9_config.scale_back_buffer_width = config_gfx->scale_back_buffer_width;
    d3d9_config.scale_back_buffer_height = config_gfx->scale_back_buffer_height;
    d3d9_config.scale_back_buffer_filter = config_gfx->scale_back_buffer_filter;
    d3d9_config.forced_refresh_rate = config_gfx->forced_refresh_rate;
    d3d9_config.device_adapter = config_gfx->device_adapter;

    iidxhook_util_d3d9_configure(&d3d9_config);

    hook_d3d9_init(iidxhook_d3d9_handlers, lengthof(iidxhook_d3d9_handlers));
}

struct iidxhook8_config_io iidxhook8_config_io;
struct camhook_config_cam config_cam;

static struct bio2emu_port bio2_emu = {
    .port = "COM4",
    .wport = L"COM4",
    .dispatcher = bio2_emu_bi2a_dispatch_request,
};

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    struct cconfig *config;

    struct iidxhook_config_gfx config_gfx;

    // log_server_init is required due to IO occuring in a non avs_thread
    log_server_init();

    log_info("-------------------------------------------------------------");
    log_info("--------------- Begin iidxhook dll_entry_init ---------------");
    log_info("-------------------------------------------------------------");

    config = cconfig_init();

    iidxhook_config_gfx_init(config);
    iidxhook8_config_io_init(config);
    camhook_config_cam_init(config, 2);

    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK8_INFO_HEADER "\n" IIDXHOOK8_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        log_server_fini();
        exit(EXIT_FAILURE);
    }

    iidxhook_config_gfx_get(&config_gfx, config);
    iidxhook8_config_io_get(&iidxhook8_config_io, config);
    camhook_config_cam_get(&config_cam, config, 2);

    cconfig_finit(config);

    log_info(IIDXHOOK8_INFO_HEADER);
    log_info("Initializing iidxhook...");

    iidxhook8_setup_d3d9_hooks(&config_gfx);

    /* Start up IIDXIO.DLL */
    if (!iidxhook8_config_io.disable_bio2_emu) {
        log_info("Starting IIDX IO backend");
        iidx_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        if (!iidx_io_init(
                avs_thread_create, avs_thread_join, avs_thread_destroy)) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    }

    /* Start up EAMIO.DLL */
    if (!iidxhook8_config_io.disable_card_reader_emu) {
        log_misc("Initializing card reader backend");
        eam_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        if (!eam_io_init(
                avs_thread_create, avs_thread_join, avs_thread_destroy)) {
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

    if (!iidxhook8_config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");
        eam_io_fini();
    }

    if (!iidxhook8_config_io.disable_bio2_emu) {
        log_misc("Shutting down IIDX IO backend");
        iidx_io_fini();
    }

    log_server_fini();

    return result;
}

/**
 * Hook library CB/Rootage
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    log_to_external(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    acp_hook_init();
    adapter_hook_init();

end:
    return TRUE;
}

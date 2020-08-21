#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/sdvxio.h"

#include "cconfig/cconfig-hook.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/config-adapter.h"
#include "hooklib/rs232.h"

#include "bio2emu/emu.h"

#include "sdvxhook2/acio.h"
#include "sdvxhook2/bi2a.h"
#include "sdvxhook2/config-io.h"

#include "camhook/cam.h"
#include "camhook/config-cam.h"

#include "d3d9exhook/config-gfx.h"
#include "d3d9exhook/d3d9ex.h"

#include "imports/avs.h"

#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"

#define SDVXHOOK2_INFO_HEADER \
    "sdvxhook for VW"         \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
#define SDVXHOOK2_CMD_USAGE \
    "Usage: launcher.exe -K sdvxhook2.dll <soundvoltex.dll> [options...]"

struct sdvxhook2_config_io config_io;
struct camhook_config_cam config_cam;
struct d3d9exhook_config_gfx config_gfx;
struct hooklib_config_adapter config_adapter;

static struct bio2emu_port bio2_emu = {
    .port = "COM4",
    .wport = L"\\\\.\\COM4",
    .dispatcher = bio2_emu_bi2a_dispatch_request,
};

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    struct cconfig *config;

    log_info("--- Begin sdvxhook dll_entry_init ---");

    config = cconfig_init();

    sdvxhook2_config_io_init(config);
    d3d9exhook_config_gfx_init(config);
    camhook_config_cam_init(config, 1);
    hooklib_config_adapter_init(config);

    if (!cconfig_hook_config_init(
            config,
            SDVXHOOK2_INFO_HEADER "\n" SDVXHOOK2_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    sdvxhook2_config_io_get(&config_io, config);
    camhook_config_cam_get(&config_cam, config, 1);
    d3d9exhook_config_gfx_get(&config_gfx, config);
    hooklib_config_adapter_get(&config_adapter, config);

    cconfig_finit(config);

    log_info(SDVXHOOK2_INFO_HEADER);
    log_info("Initializing sdvxhook2...");

    d3d9ex_configure(&config_gfx);

    /* Start up sdvxio.DLL */
    if (!config_io.disable_bio2_emu) {
        log_info("Starting sdvx IO backend");
        sdvx_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        if (!sdvx_io_init(
                avs_thread_create, avs_thread_join, avs_thread_destroy)) {
            log_fatal("Initializing sdvx IO backend failed");
        }
    }

    /* Start up EAMIO.DLL */
    if (!config_io.disable_card_reader_emu) {
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
    iohook_push_handler(ac_io_port_dispatch_irp);
    iohook_push_handler(bio2emu_port_dispatch_irp);

    rs232_hook_init();
    rs232_hook_limit_hooks();

    if (!config_io.disable_bio2_emu) {
        bio2emu_init();
        bio2_emu_bi2a_init(&bio2_emu, config_io.disable_poll_limiter, config_io.force_headphones);
    }

    if (!config_io.disable_card_reader_emu) {
        ac_io_port_init();
    }

    // camera hooks
    if (!config_cam.disable_emu) {
        camhook_init(&config_cam);
    }

    adapter_hook_override(config_adapter.override_ip);

    log_info("--- End sdvxhook dll_entry_init ---");

    return app_hook_invoke_init(sidcode, param);
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    if (!config_cam.disable_emu) {
        camhook_fini();
    }

    if (!config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");
        eam_io_fini();
    }

    if (!config_io.disable_bio2_emu) {
        log_misc("Shutting down sdvx IO backend");
        sdvx_io_fini();
    }

    return result;
}

/**
 * Hook library SDVX5+
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
    d3d9ex_hook_init();

end:
    return TRUE;
}

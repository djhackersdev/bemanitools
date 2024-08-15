#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cconfig/cconfig-hook.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/config-adapter.h"
#include "hooklib/memfile.h"
#include "hooklib/rs232.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/sdvx.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "bio2emu/emu.h"

#include "sdvxhook2/acio.h"
#include "sdvxhook2/bi2a.h"
#include "sdvxhook2/config-io.h"
#include "sdvxhook2/nvapi.h"
#include "sdvxhook2/power.h"

#include "camhook/cam.h"
#include "camhook/config-cam.h"

#include "d3d9exhook/config-gfx.h"
#include "d3d9exhook/d3d9ex.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/str.h"

#define SDVXHOOK2_INFO_HEADER \
    "sdvxhook for VW"         \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
#define SDVXHOOK2_CMD_USAGE \
    "Usage: launcher.exe -K sdvxhook2.dll <soundvoltex.dll> [options...]"

struct sdvxhook2_config_io config_io;
struct camhook_config_cam config_cam;
struct d3d9exhook_config_gfx config_gfx;
struct hooklib_config_adapter config_adapter;

static module_io_t *_sdvxhook2_module_io_sdvx;
static module_io_t *_sdvxhook2_module_io_eam;

static struct bio2emu_port bio2_emu = {
    .port = "COM4",
    .wport = L"\\\\.\\COM4",
    .dispatcher = bio2_emu_bi2a_dispatch_request,
};

static void attach_dest_fd_intercept(const char *sidcode)
{
    char region = sidcode[3];

    if (region == 'X') {
        region = 'J';
    }

    char target_file[8] = "\\x.dest";
    target_file[1] = tolower(region);

    // can only capture these by ending path due to /dev/raw being mountable
    memfile_hook_add_fd(target_file, ENDING_MATCH, NULL, 0);
}

static void _sdvxhook2_io_sdvx_init(module_io_t **module)
{
    bt_io_sdvx_api_t api;

    module_io_ext_load_and_init(
        "sdvxio.dll", "bt_module_io_sdvx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_sdvx_api_set(&api);
}

static void _sdvxhook2_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool
_sdvxhook2_main_init(HMODULE game_module, const bt_core_config_t *config_)
{
    struct cconfig *config;

    log_info("--- Begin sdvxhook dll_entry_init ---");

    // TODO sidcode back-propagation required like jubeat, figure out
    char *sidcode = NULL;

    log_assert(sidcode);

    config = cconfig_init();

    sdvxhook2_config_io_init(config);
    d3d9exhook_config_gfx_init(config);
    camhook_config_cam_init(config, 1, false);
    hooklib_config_adapter_init(config);

    if (!cconfig_hook_config_init(
            config,
            SDVXHOOK2_INFO_HEADER "\n" SDVXHOOK2_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    sdvxhook2_config_io_get(&config_io, config);
    camhook_config_cam_get(&config_cam, config, 1, false);
    d3d9exhook_config_gfx_get(&config_gfx, config);
    hooklib_config_adapter_get(&config_adapter, config);

    cconfig_finit(config);

    log_info(SDVXHOOK2_INFO_HEADER);
    log_info("Initializing sdvxhook2...");

    acp_hook_init();
    adapter_hook_init();
    d3d9ex_hook_init();

    d3d9ex_configure(&config_gfx);

    /* Start up sdvxio.DLL */
    if (!config_io.disable_bio2_emu) {
        log_info("Starting sdvx IO backend");

        _sdvxhook2_io_sdvx_init(&_sdvxhook2_module_io_sdvx);

        if (!bt_io_sdvx_init()) {
            log_fatal("Initializing sdvx IO backend failed");
        }
    }

    /* Start up EAMIO.DLL */
    if (!config_io.disable_card_reader_emu) {
        log_misc("Initializing card reader backend");

        _sdvxhook2_io_eam_init(&_sdvxhook2_module_io_eam);

        if (!bt_io_eam_init()) {
            log_fatal("Initializing card reader backend failed");
        }
    }

    /* iohooks are okay, even if emu is diabled since the fake handlers won't be
     * used */
    iohook_push_handler(ac_io_port_dispatch_irp);
    iohook_push_handler(bio2emu_port_dispatch_irp);

    if (!config_io.disable_file_hooks) {
        memfile_hook_init();
        iohook_push_handler(memfile_hook_dispatch_irp);
        attach_dest_fd_intercept(sidcode);
    }

    rs232_hook_init();
    rs232_hook_limit_hooks();

    if (!config_io.disable_bio2_emu) {
        bio2emu_init();
        bio2_emu_bi2a_init(
            &bio2_emu,
            config_io.disable_poll_limiter,
            config_io.force_headphones);
    }

    if (!config_io.disable_card_reader_emu) {
        ac_io_port_init(config_io.com1_card_reader);
    }

    if (!config_cam.disable_emu) {
        camhook_init(&config_cam);
    }

    if (!config_io.disable_power_hooks) {
        powerhook_init();
    }

    if (!config_io.disable_nvapi_hooks) {
        nvapihook_init();
    }

    adapter_hook_override(config_adapter.override_ip);

    log_info("--- End sdvxhook dll_entry_init ---");

    return true;
}

static void _sdvxhook2_main_fini()
{
    if (!config_cam.disable_emu) {
        camhook_fini();
    }

    if (!config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");

        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&_sdvxhook2_module_io_eam);
    }

    if (!config_io.disable_bio2_emu) {
        log_misc("Shutting down sdvx IO backend");

        bt_io_sdvx_fini();

        bt_io_sdvx_api_clear();
        module_io_free(&_sdvxhook2_module_io_sdvx);
    }

    if (!config_io.disable_file_hooks) {
        memfile_hook_fini();
    }
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

    api->v1.main_init = _sdvxhook2_main_init;
    api->v1.main_fini = _sdvxhook2_main_fini;
}

/**
 * Hook library SDVX5+
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    return TRUE;
}

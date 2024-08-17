#define LOG_MODULE "sdvxhook2"

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

static sdvxhook2_config_io_t _sdvxhook2_config_io;
static camhook_config_cam_t _sdvxhook2_config_cam;
static d3d9exhook_config_gfx_t _sdvxhook2_config_gfx;
static hooklib_config_adapter_t _sdvxhook2_config_adapter;

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
_sdvxhook2_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    log_info("sdvxhook for Vivid Wave and EXCEED GEAR");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    // TODO sidcode back-propagation required like jubeat, figure out, crash for now
    char *sidcode = NULL;
    log_fatal("DEV TODO: sidcode back-propagation missing");
    log_assert(sidcode);

    sdvxhook2_config_io_get(config, &_sdvxhook2_config_io);
    camhook_config_cam_get(config, &_sdvxhook2_config_cam, 1, false);
    d3d9exhook_config_gfx_get(config, &_sdvxhook2_config_gfx);
    hooklib_config_adapter_get(config, &_sdvxhook2_config_adapter);

    acp_hook_init();
    adapter_hook_init();
    d3d9ex_hook_init();

    d3d9ex_configure(&_sdvxhook2_config_gfx);

    /* Start up sdvxio.DLL */
    if (!_sdvxhook2_config_io.disable_bio2_emu) {
        log_info("Starting sdvx IO backend");

        _sdvxhook2_io_sdvx_init(&_sdvxhook2_module_io_sdvx);

        if (!bt_io_sdvx_init()) {
            log_fatal("Initializing sdvx IO backend failed");
        }
    }

    /* Start up EAMIO.DLL */
    if (!_sdvxhook2_config_io.disable_card_reader_emu) {
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

    if (!_sdvxhook2_config_io.disable_file_hooks) {
        memfile_hook_init();
        iohook_push_handler(memfile_hook_dispatch_irp);
        attach_dest_fd_intercept(sidcode);
    }

    rs232_hook_init();
    rs232_hook_limit_hooks();

    if (!_sdvxhook2_config_io.disable_bio2_emu) {
        bio2emu_init();
        bio2_emu_bi2a_init(
            &bio2_emu,
            _sdvxhook2_config_io.disable_poll_limiter,
            _sdvxhook2_config_io.force_headphones);
    }

    if (!_sdvxhook2_config_io.disable_card_reader_emu) {
        ac_io_port_init(_sdvxhook2_config_io.com1_card_reader);
    }

    if (!_sdvxhook2_config_cam.disable_emu) {
        camhook_init(&_sdvxhook2_config_cam);
    }

    if (!_sdvxhook2_config_io.disable_power_hooks) {
        powerhook_init();
    }

    if (!_sdvxhook2_config_io.disable_nvapi_hooks) {
        nvapihook_init();
    }

    adapter_hook_override(_sdvxhook2_config_adapter.override_ip);

    log_info("--- End sdvxhook dll_entry_init ---");

    return true;
}

static void _sdvxhook2_main_fini()
{
    if (!_sdvxhook2_config_cam.disable_emu) {
        camhook_fini();
    }

    if (!_sdvxhook2_config_io.disable_card_reader_emu) {
        log_misc("Shutting down card reader backend");

        bt_io_eam_fini();

        bt_io_eam_api_clear();
        module_io_free(&_sdvxhook2_module_io_eam);
    }

    if (!_sdvxhook2_config_io.disable_bio2_emu) {
        log_misc("Shutting down sdvx IO backend");

        bt_io_sdvx_fini();

        bt_io_sdvx_api_clear();
        module_io_free(&_sdvxhook2_module_io_sdvx);
    }

    if (!_sdvxhook2_config_io.disable_file_hooks) {
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

    api->v1.main_init = _sdvxhook2_main_init;
    api->v1.main_fini = _sdvxhook2_main_fini;
}
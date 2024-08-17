#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/rs232.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/sdvx.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "sdvxhook2-cn/acio.h"
#include "sdvxhook2-cn/config-cn.h"
#include "sdvxhook2-cn/unis-version.h"

#include "camhook/cam.h"
#include "camhook/config-cam.h"

#include "d3d9exhook/config-gfx.h"
#include "d3d9exhook/d3d9ex.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "util/str.h"

static sdvxhook2_cn_config_t _sdvxhook2_cn_config;
static camhook_config_cam_t _sdvxhook2_cn_config_cam;
static d3d9exhook_config_gfx_t _sdvxhook2_cn_config_gfx;

static module_io_t *_sdvxhook2_cn_module_io_sdvx;

static void _sdvxhook2_cn_io_sdvx_init(module_io_t **module)
{
    bt_io_sdvx_api_t api;

    module_io_ext_load_and_init(
        "sdvxio.dll", "bt_module_io_sdvx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_sdvx_api_set(&api);
}

static bool
_sdvxhook2_cn_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    log_info("sdvxhook for Vivid Wave CN");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    sdvxhook2_cn_config_get(config, &_sdvxhook2_cn_config);
    camhook_config_cam_get(config, &_sdvxhook2_cn_config_cam, 1, false);
    d3d9exhook_config_gfx_get(config, &_sdvxhook2_cn_config_gfx);

    acp_hook_init();
    adapter_hook_init();
    d3d9ex_hook_init();

    d3d9ex_configure(&_sdvxhook2_cn_config_gfx);

    /* Start up sdvxio.DLL */
    if (!_sdvxhook2_cn_config.disable_io_emu) {
        log_info("Starting sdvx IO backend");

        _sdvxhook2_cn_io_sdvx_init(&_sdvxhook2_cn_module_io_sdvx);

        if (!bt_io_sdvx_init()) {
            log_fatal("Initializing sdvx IO backend failed");
        }
    }

    /* iohooks are okay, even if emu is diabled since the fake handlers won't be
     * used */
    iohook_push_handler(ac_io_port_dispatch_irp);

    rs232_hook_init();
    rs232_hook_limit_hooks();

    if (!_sdvxhook2_cn_config.disable_io_emu) {
        ac_io_port_init();
    }

    // camera hooks
    if (!_sdvxhook2_cn_config_cam.disable_emu) {
        camhook_init(&_sdvxhook2_cn_config_cam);
    }

    unis_version_hook_init(_sdvxhook2_cn_config.unis_path);

    return true;
}

static void _sdvxhook2_cn_main_fini()
{
    if (!_sdvxhook2_cn_config_cam.disable_emu) {
        camhook_fini();
    }

    if (!_sdvxhook2_cn_config.disable_io_emu) {
        log_misc("Shutting down sdvx IO backend");

        bt_io_sdvx_fini();

        bt_io_sdvx_api_clear();
        module_io_free(&_sdvxhook2_cn_module_io_sdvx);
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

    api->v1.main_init = _sdvxhook2_cn_main_init;
    api->v1.main_fini = _sdvxhook2_cn_main_fini;
}
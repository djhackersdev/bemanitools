#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "avs-ext/log.h"
#include "avs-ext/thread.h"

#include "cconfig/cconfig-hook.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"

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

#include "imports/avs.h"

#include "util/str.h"

#define SDVXHOOK2_CN_INFO_HEADER \
    "sdvxhook for VW CN"         \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV) "\n"
#define SDVXHOOK2_CN_CMD_USAGE \
    "Usage: launcher.exe -K sdvxhook2.dll <soundvoltex.dll> [options...]"

struct sdvxhook2_cn_config config_cn;
struct camhook_config_cam config_cam;
struct d3d9exhook_config_gfx config_gfx;

static module_io_t *_sdvxhook2_cn_module_io_sdvx;

static void _sdvxhook2_cn_io_sdvx_init(module_io_t **module)
{
    bt_io_sdvx_api_t api;

    module_io_ext_load_and_init(
        "sdvxio.dll", "bt_module_io_sdvx_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_sdvx_api_set(&api);
}

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    struct cconfig *config;

    log_info("--- Begin sdvxhook dll_entry_init ---");

    config = cconfig_init();

    sdvxhook2_cn_config_init(config);
    d3d9exhook_config_gfx_init(config);
    camhook_config_cam_init(config, 1);

    if (!cconfig_hook_config_init(
            config,
            SDVXHOOK2_CN_INFO_HEADER "\n" SDVXHOOK2_CN_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    sdvxhook2_cn_config_get(&config_cn, config);
    camhook_config_cam_get(&config_cam, config, 1);
    d3d9exhook_config_gfx_get(&config_gfx, config);

    cconfig_finit(config);

    log_info(SDVXHOOK2_CN_INFO_HEADER);
    log_info("Initializing sdvxhook2-cn...");

    d3d9ex_configure(&config_gfx);

    /* Start up sdvxio.DLL */
    if (!config_cn.disable_io_emu) {
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

    if (!config_cn.disable_io_emu) {
        ac_io_port_init();
    }

    // camera hooks
    if (!config_cam.disable_emu) {
        camhook_init(&config_cam);
    }

    unis_version_hook_init(config_cn.unis_path);

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

    if (!config_cn.disable_io_emu) {
        log_misc("Shutting down sdvx IO backend");

        bt_io_sdvx_fini();

        bt_io_sdvx_api_clear();
        module_io_free(&_sdvxhook2_cn_module_io_sdvx);
    }

    return result;
}

/**
 * Hook library SDVX5 CN
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    // Use AVS APIs
    avs_ext_log_core_api_set();
    avs_ext_thread_core_api_set();

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    acp_hook_init();
    adapter_hook_init();
    d3d9ex_hook_init();

end:
    return TRUE;
}

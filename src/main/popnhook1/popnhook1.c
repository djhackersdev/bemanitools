#include <stdio.h>

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "ezusb-emu/node-security-plug.h"

#include "hook/d3d9.h"

#include "hooklib/adapter.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/eam.h"
#include "iface-io/popn.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "popnhook1/avs-boot.h"
#include "popnhook1/config-eamuse.h"
#include "popnhook1/config-gfx.h"
#include "popnhook1/config-sec.h"
#include "popnhook1/d3d9.h"
#include "popnhook1/filesystem.h"

#include "popnhook-util/acio.h"
#include "popnhook-util/mixer.h"

#include "util/cmdline.h"
#include "util/defs.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "popnhook-util/acio.h"

#include "ezusb2-popn-emu/msg.h"

#include "hooklib/rs232.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "security/rp-sign-key.h"

static module_io_t *popnhook_module_io_popn;
static module_io_t *popnhook_module_io_eam;

static void _popnhook1_io_popn_init(module_io_t **module)
{
    bt_io_popn_api_t api;

    module_io_ext_load_and_init(
        "popnio.dll", "bt_module_io_popn_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_popn_api_set(&api);
}

static void _popnhook1_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static void popnhook_setup_d3d9_hooks(
    const popnhook1_config_gfx_t *config_gfx, const bool texture_usage_fix)
{
    struct popnhook1_d3d9_config d3d9_config;

    popnhook1_d3d9_init_config(&d3d9_config);

    d3d9_config.windowed = config_gfx->windowed;
    d3d9_config.framed = config_gfx->framed;
    d3d9_config.override_window_width = config_gfx->window_width;
    d3d9_config.override_window_height = config_gfx->window_height;
    d3d9_config.texture_usage_fix = texture_usage_fix;

    popnhook1_d3d9_init();
    popnhook1_d3d9_configure(&d3d9_config);
}

static bool
_popnhook1_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    popnhook1_config_eamuse_t config_eamuse;
    popnhook1_config_gfx_t config_gfx;
    popnhook1_config_sec_t config_sec;

    log_info("popnhook1 for pop'n music 15, 16, 17, 18");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    popnhook1_config_gfx_get(config, &config_gfx);
    popnhook1_config_eamuse_get(config, &config_eamuse);
    popnhook1_config_sec_get(config, &config_sec);

    popnhook_setup_d3d9_hooks(
        &config_gfx,
        // pop'n music 16 requires a patch for the texture usage to not crash on
        // newer Windows
        memcmp(
            config_sec.black_plug_mcode.game,
            SECURITY_MCODE_GAME_POPN_16,
            sizeof(config_sec.black_plug_mcode.game)) == 0);

    popnhook1_avs_boot_init();
    popnhook1_avs_boot_set_eamuse_addr(&config_eamuse.server);

    /* Round plug security */

    ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
        &config_sec.black_plug_mcode);
    ezusb_iidx_emu_node_security_plug_set_plug_white_mcode(
        &security_mcode_eamuse);
    ezusb_iidx_emu_node_security_plug_set_plug_black_sign_key(
        &security_rp_sign_key_black_popn);
    ezusb_iidx_emu_node_security_plug_set_plug_white_sign_key(
        &security_rp_sign_key_white_eamuse);

    ezusb_iidx_emu_node_security_plug_set_pcbid(&config_eamuse.pcbid);
    ezusb_iidx_emu_node_security_plug_set_eamid(&config_eamuse.eamid);

    /* Start up POPNIO.DLL */

    log_info("Starting pop'n IO backend");

    _popnhook1_io_popn_init(&popnhook_module_io_popn);

    if (!bt_io_popn_init()) {
        log_fatal("Initializing pop'n IO backend failed");
    }

    /* Start up EAMIO.DLL */

    log_misc("Initializing card reader backend");

    _popnhook1_io_eam_init(&popnhook_module_io_eam);

    if (!bt_io_eam_init()) {
        log_fatal("Initializing card reader backend failed");
    }

    iohook_push_handler(ezusb2_emu_device_dispatch_irp);
    iohook_push_handler(popnhook_acio_dispatch_irp);

    hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
    ezusb2_emu_device_hook_init(ezusb2_popn_emu_msg_init());

    rs232_hook_init();

    popnhook_acio_init(true);

    adapter_hook_init();
    filesystem_init();
    popnhook_mixer_hook_init();

    return true;
}

static void _popnhook1_main_fini()
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

    api->v1.main_init = _popnhook1_main_init;
    api->v1.main_fini = _popnhook1_main_fini;
}
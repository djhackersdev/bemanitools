#include <windows.h>

#include <stdbool.h>

#include "ddrhook-util/_com4.h"
#include "ddrhook-util/extio.h"
#include "ddrhook-util/p3io.h"
#include "ddrhook-util/spike.h"
#include "ddrhook-util/usbmem.h"

#include "ddrhook1/avs-boot.h"
#include "ddrhook1/config-ddrhook1.h"
#include "ddrhook1/config-eamuse.h"
#include "ddrhook1/config-gfx.h"
#include "ddrhook1/config-security.h"
#include "ddrhook1/filesystem.h"
#include "ddrhook1/master.h"

#include "ddrhook-util/gfx.h"

#include "hook/iohook.h"

#include "hooklib/rs232.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "iface-io/ddr.h"
#include "iface-io/eam.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "p3ioemu/emu.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

#include "security/rp-sign-key.h"

bool standard_def;
bool _15khz;

static const hook_d3d9_irp_handler_t ddrhook1_d3d9_handlers[] = {
    gfx_d3d9_irp_handler,
};

static module_io_t *_ddrhook1_module_io_ddr;
static module_io_t *_ddrhook1_module_io_eam;

static void _ddrhook1_io_ddr_init(module_io_t **module)
{
    bt_io_ddr_api_t api;

    module_io_ext_load_and_init(
        "ddrio.dll", "bt_module_io_ddr_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_ddr_api_set(&api);
}

static void _ddrhook1_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool
_ddrhook1_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    bool ok;
    ddrhook1_config_ddrhook1_t config_ddrhook1;
    ddrhook1_config_eamuse_t config_eamuse;
    ddrhook1_config_gfx_t config_gfx;
    ddrhook1_config_security_t config_security;

    log_info("ddrhook1 for DDR X");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    ddrhook1_config_ddrhook1_get(config, &config_ddrhook1);
    ddrhook1_config_eamuse_get(config, &config_eamuse);
    ddrhook1_config_gfx_get(config, &config_gfx);
    ddrhook1_config_security_get(config, &config_security);

    standard_def = config_ddrhook1.standard_def;
    _15khz = config_ddrhook1.use_15khz;

    ddrhook1_master_insert_hooks(NULL);
    ddrhook1_filesystem_hook_init();

    hook_d3d9_init(
        ddrhook1_d3d9_handlers, lengthof(ddrhook1_d3d9_handlers));

    ddrhook1_filesystem_hook_init();

    ddrhook1_avs_boot_init();
    ddrhook1_avs_boot_set_eamuse_addr(&config_eamuse.server);

    iohook_push_handler(p3io_emu_dispatch_irp);
    iohook_push_handler(extio_dispatch_irp);
    iohook_push_handler(spike_dispatch_irp);
    iohook_push_handler(usbmem_dispatch_irp);

    if (config_ddrhook1.use_com4_emu) {
        /* See ddrhook-util/p3io.c for details. */
        iohook_push_handler(com4_dispatch_irp);
    }

    if (config_gfx.windowed) {
        gfx_set_windowed();
    }

    rs232_hook_init();

    p3io_ddr_init_with_plugs(
        &config_security.mcode,
        &config_eamuse.pcbid,
        &config_eamuse.eamid,
#if AVS_VERSION >= 1002
        &security_rp_sign_key_black_ddrx2,
#else
        &security_rp_sign_key_black_ddrx,
#endif
        &security_rp_sign_key_white_eamuse);
    extio_init();
    usbmem_init(
        config_ddrhook1.usbmem_path_p1,
        config_ddrhook1.usbmem_path_p2,
        config_ddrhook1.usbmem_enabled);
    spike_init();
    com4_init();

    log_info("Initializing DDR IO backend");

    _ddrhook1_io_ddr_init(&_ddrhook1_module_io_ddr);

    ok = bt_io_ddr_init();

    if (!ok) {
        log_fatal("Couldn't initialize DDR IO backend");
        return false;
    }

    if (config_ddrhook1.use_com4_emu) {
        log_info("Initializing card reader backend");

        _ddrhook1_io_eam_init(&_ddrhook1_module_io_eam);

        ok = bt_io_eam_init();

        if (!ok) {
            log_fatal("Couldn't initialize card reader backend");
            return false;
        }
    }

    return true;
}

static void _ddrhook1_main_fini()
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

    api->v1.main_init = _ddrhook1_main_init;
    api->v1.main_fini = _ddrhook1_main_fini;
}
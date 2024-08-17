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

#include "iface-io/eam.h"
#include "iface-io/jb.h"

#include "jbhook1/avs-boot.h"
#include "jbhook1/config-eamuse.h"
#include "jbhook1/config-gfx.h"
#include "jbhook1/config-security.h"
#include "jbhook1/gfx.h"
#include "jbhook1/log-gftools.h"

#include "jbhook-util/acio.h"
#include "jbhook-util/eamuse.h"

#include "jbhook-util-p3io/mixer.h"
#include "jbhook-util-p3io/p3io.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "p3ioemu/devmgr.h"
#include "p3ioemu/emu.h"

#include "sdk/module/core/config.h"
#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/hook.h"

static module_io_t *jbhook_module_io_jb;
static module_io_t *jbhook_module_io_eam;

static void _jbhook1_io_jb_init(module_io_t **module)
{
    bt_io_jb_api_t api;

    module_io_ext_load_and_init("jbio.dll", "bt_module_io_jb_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_jb_api_set(&api);
}

static void _jbhook1_io_eam_init(module_io_t **module)
{
    bt_io_eam_api_t api;

    module_io_ext_load_and_init(
        "eamio.dll", "bt_module_io_eam_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_eam_api_set(&api);
}

static bool
_jbhook1_main_init(HMODULE game_module, const bt_core_config_t *config)
{
    jbhook1_config_gfx_t config_gfx;
    jbhook1_config_eamuse_t config_eamuse;
    jbhook1_config_security_t config_security;

    log_info("jbhook for jubeat and jubeat ripples");
    log_info("build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV));

    jbhook1_config_gfx_get(config, &config_gfx);
    jbhook1_config_eamuse_get(config, &config_eamuse);
    jbhook1_config_security_get(config, &config_security);

    acp_hook_init();
    adapter_hook_init();

    // TODO this won't work because it modifies the parameters of the mwindow_create
    // call which is also used for bootstrapping this function (_jbhook1_main_init)
    // Introducing the additional abstraction and glue code to further separate
    // concerns, a different function before mwindow_create must be used in
    // dllmain to bootstrap the main hooking code here, so the mwindow_create
    // hook can be set up in advance and then be invoked by the game
    jbhook1_gfx_init(&config_gfx);

    jbhook1_avs_boot_init();
    jbhook1_avs_boot_set_eamuse_addr(&config_eamuse.server);
    jbhook1_log_gftools_init();
    jbhook_util_mixer_hook_init();

    log_info("Starting up jubeat IO backend");

    _jbhook1_io_jb_init(&jbhook_module_io_jb);

    if (!bt_io_jb_init()) {
        log_fatal("Initializing jb IO backend failed");
    }

    log_info("Starting up card reader backend");

    _jbhook1_io_eam_init(&jbhook_module_io_eam);

    if (!bt_io_eam_init()) {
        log_fatal("Initializing card reader backend failed");
    }

    jbhook_util_eamuse_hook_init();

    iohook_push_handler(p3io_emu_dispatch_irp);
    iohook_push_handler(jbhook_util_ac_io_port_dispatch_irp);

    rs232_hook_init();
    jbhook_util_ac_io_port_init(L"COM1");
    jbhook_util_ac_io_set_iccb();

    p3io_setupapi_insert_hooks(NULL);
    jbhook_util_p3io_init(
        &config_security.mcode, &config_eamuse.pcbid, &config_eamuse.eamid);

    return true;
}

static void _jbhook1_main_fini()
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

    api->v1.main_init = _jbhook1_main_init;
    api->v1.main_fini = _jbhook1_main_fini;
}
#include <stdio.h>

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "bemanitools/eamio.h"
#include "bemanitools/popnio.h"

#include "cconfig/cconfig-hook.h"

#include "ezusb-emu/node-security-plug.h"

#include "hook/d3d9.h"
#include "hook/table.h"

#include "hooklib/adapter.h"

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
#include "util/log.h"
#include "util/thread.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "iidxhook-util/acio.h"

#include "ezusb2-popn-emu/msg.h"

#include "hooklib/rs232.h"

#include "security/rp-sign-key.h"

#define POPNHOOK1_INFO_HEADER                  \
    "popnhook1 for pop'n music 15, 16, 17, 18" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define POPNHOOK1_CMD_USAGE \
    "Usage: inject.exe popnhook1.dll <popn.exe> [options...]"

static DWORD STDCALL my_GetStartupInfoA(LPSTARTUPINFOA lpStartupInfo);
static DWORD(STDCALL *real_GetStartupInfoA)(LPSTARTUPINFOA lpStartupInfo);

static bool popnhook1_init_check;

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "GetStartupInfoA",
        .patch = my_GetStartupInfoA,
        .link = (void **) &real_GetStartupInfoA,
    },
};

static void popnhook_setup_d3d9_hooks(
    const struct popnhook1_config_gfx *config_gfx, const bool texture_usage_fix)
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

static DWORD STDCALL my_GetStartupInfoA(LPSTARTUPINFOA lpStartupInfo)
{
    struct cconfig *config;

    struct popnhook1_config_eamuse config_eamuse;
    struct popnhook1_config_gfx config_gfx;
    struct popnhook1_config_sec config_sec;

    if (popnhook1_init_check)
        goto skip;

    popnhook1_init_check = true;

    log_info("-------------------------------------------------------------");
    log_info("----------------- Start popnhook1 hook init -----------------");
    log_info("-------------------------------------------------------------");

    config = cconfig_init();

    popnhook1_config_eamuse_init(config);
    popnhook1_config_gfx_init(config);
    popnhook1_config_sec_init(config);

    if (!cconfig_hook_config_init(
            config,
            POPNHOOK1_INFO_HEADER "\n" POPNHOOK1_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    popnhook1_config_gfx_get(&config_gfx, config);
    popnhook1_config_eamuse_get(&config_eamuse, config);
    popnhook1_config_sec_get(&config_sec, config);

    cconfig_finit(config);

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
    popn_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!popn_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing pop'n IO backend failed");
    }

    /* Start up EAMIO.DLL */

    log_misc("Initializing card reader backend");
    eam_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

    if (!eam_io_init(thread_create, thread_join, thread_destroy)) {
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

    log_info("-------------------------------------------------------------");
    log_info("------------------ End popnhook1 hook init ------------------");
    log_info("-------------------------------------------------------------");

skip:
    return real_GetStartupInfoA(lpStartupInfo);
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    log_to_writer(log_writer_debug, NULL);

    hook_table_apply(
        NULL, "kernel32.dll", init_hook_syms, lengthof(init_hook_syms));

    return TRUE;
}

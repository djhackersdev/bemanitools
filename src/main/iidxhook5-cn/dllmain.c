#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "ezusb-iidx-emu/node-security-plug.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "ezusb2-iidx-emu/msg.h"

#include "hook/d3d9.h"
#include "hook/table.h"

#include "hooklib/acp.h"
#include "hooklib/setupapi.h"

#include "iidxhook5-cn/avs-boot.h"
#include "iidxhook5-cn/path.h"

#include "iidxhook-util/config-eamuse.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/config-io.h"
#include "iidxhook-util/config-sec.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/settings.h"

#include "security/rp-sign-key.h"

#include "imports/avs.h"

#include "util/log.h"

#define IIDXHOOK5_CN_INFO_HEADER \
    "iidxhook for tricoro CN"   \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK5_CN_CMD_USAGE \
    "Usage: inject.exe iidxhook5-cn.dll <bm2dx.exe> [options...]"

static ATOM WINAPI my_RegisterClassA(const WNDCLASSA *lpWndClass);
static ATOM(WINAPI *real_RegisterClassA)(const WNDCLASSA *lpWndClass);
static bool iidxhook_init_check;

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static const struct hook_symbol init_hook_user32_syms[] = {
    {.name = "RegisterClassA",
     .patch = my_RegisterClassA,
     .link = (void **) &real_RegisterClassA},
};

static struct iidxhook_config_io config_io;

static void
iidxhook5_cn_setup_d3d9_hooks(const struct iidxhook_config_gfx *config_gfx)
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
    /* Required for GOLD (and newer?) to not crash with NVIDIA cards */
    d3d9_config.iidx14_to_19_nvidia_fix = true;
    d3d9_config.iidx18_and_19_diagonal_tearing_fix = config_gfx->diagonal_tearing_fix;

    iidxhook_util_d3d9_configure(&d3d9_config);

    hook_d3d9_init(iidxhook_d3d9_handlers, lengthof(iidxhook_d3d9_handlers));
}

/**
 * OpenProcess is basically used for hooks in exe games,
 * but since it does not exist in this game, RegisterClassA is used instead.
 */
static ATOM WINAPI my_RegisterClassA(const WNDCLASSA *lpWndClass)
{
    struct cconfig *config;

    struct iidxhook_util_config_eamuse config_eamuse;
    struct iidxhook_config_gfx config_gfx;
    struct iidxhook_config_sec config_sec;

    if (iidxhook_init_check) {
        return real_RegisterClassA(lpWndClass);
    }

    iidxhook_init_check = true;

    log_info("-------------------------------------------------------------");
    log_info("------------- Begin iidxhook my_RegisterClassA --------------");
    log_info("-------------------------------------------------------------");

    config = cconfig_init();

    iidxhook_util_config_eamuse_init(config);
    iidxhook_config_gfx_init(config);
    iidxhook_config_io_init(config);
    iidxhook_config_sec_init(config);

    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK5_CN_INFO_HEADER "\n" IIDXHOOK5_CN_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    iidxhook_util_config_eamuse_get(&config_eamuse, config);
    iidxhook_config_gfx_get(&config_gfx, config);
    iidxhook_config_io_get(&config_io, config);
    iidxhook_config_sec_get(&config_sec, config);

    cconfig_finit(config);

    log_info(IIDXHOOK5_CN_INFO_HEADER);
    log_info("Initializing iidxhook...");

    /**
     * This game is using a black round plug for game license management instead of a black usb dongle.
     * No white dongle hooks applies since the game does not have network functionality.
     * Also, card readers are not used/checked; no card reader hooks required.
     */
    ezusb_iidx_emu_node_security_plug_set_boot_version(
        &config_sec.boot_version);
    ezusb_iidx_emu_node_security_plug_set_boot_seeds(config_sec.boot_seeds);
    ezusb_iidx_emu_node_security_plug_set_plug_black_sign_key(
        &security_rp_sign_key_black_iidx);
    ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
        &config_sec.black_plug_mcode);
    ezusb_iidx_emu_node_security_plug_set_pcbid(&config_eamuse.pcbid);

    iidxhook5_cn_setup_d3d9_hooks(&config_gfx);

    if (!config_io.disable_io_emu) {
        log_info("Starting IIDX IO backend");

        iidx_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        if (!iidx_io_init(
                avs_thread_create, avs_thread_join, avs_thread_destroy)) {
            log_fatal("Initializing IIDX IO backend failed");
        }
    } else {
        log_info("IIDX IO emulation backend disabled");
    }

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_push_handler(ezusb2_emu_device_dispatch_irp);
    iohook_push_handler(settings_hook_dispatch_irp);

    if (!config_io.disable_io_emu) {
        hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
        ezusb2_emu_device_hook_init(ezusb2_iidx_emu_msg_init());
    }

    log_info("-------------------------------------------------------------");
    log_info("-------------- End iidxhook my_RegisterClassA ---------------");
    log_info("-------------------------------------------------------------");

    return real_RegisterClassA(lpWndClass);
}

/**
 * Hook library for tricoro CN
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    log_to_writer(log_writer_debug, NULL);

    hook_table_apply(
        NULL,
        "user32.dll",
        init_hook_user32_syms,
        lengthof(init_hook_user32_syms));

    iidxhook5_cn_path_init();
    iidxhook5_cn_avs_boot_init();
    acp_hook_init();
    settings_hook_init();

    return TRUE;
}

#include <windows.h>

#include <stdbool.h>

#include "bemanitools/ddrio.h"
#include "bemanitools/eamio.h"

#include "cconfig/cconfig-hook.h"

#include "ddrhook-util/_com4.h"
#include "ddrhook-util/extio.h"
#include "ddrhook-util/p3io.h"
#include "ddrhook-util/spike.h"
#include "ddrhook-util/usbmem.h"

#include "ddrhookx/avs-boot.h"
#include "ddrhookx/config-ddrhookx.h"
#include "ddrhookx/config-eamuse.h"
#include "ddrhookx/config-gfx.h"
#include "ddrhookx/config-security.h"
#include "ddrhookx/d3d9.h"
#include "ddrhookx/filesystem.h"
#include "ddrhookx/master.h"

#include "hook/iohook.h"
#include "hook/table.h"

#include "hooklib/app.h"
#include "hooklib/rs232.h"

#include "imports/avs.h"

#include "p3ioemu/emu.h"

#include "security/rp-sign-key.h"

#include "util/cmdline.h"
#include "util/defs.h"
#include "util/log.h"
#include "util/thread.h"

#define DDRHOOKX_INFO_HEADER \
    "ddrhookx for DDR X"    \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define DDRHOOKX_CMD_USAGE \
    "Usage: inject.exe ddrhookx.dll <ddr.exe> [options...]"

bool standard_def;
bool _15khz;

static const hook_d3d9_irp_handler_t ddrhookx_d3d9_handlers[] = {
    ddrhookx_d3d9_irp_handler,
};

static DWORD STDCALL my_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
static DWORD(STDCALL *real_GetModuleFileNameA)(HMODULE hModule, LPSTR lpFilename, DWORD nSize);

static bool ddrhookx_init_check = false;

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "GetModuleFileNameA",
        .patch = my_GetModuleFileNameA,
        .link = (void **) &real_GetModuleFileNameA,
    },
};

static void ddrhookx_setup_d3d9_hooks(
    const struct ddrhookx_config_gfx *config_gfx)
{
    struct ddrhookx_d3d9_config d3d9_config;

    d3d9_config.windowed = config_gfx->windowed;

    ddrhookx_d3d9_configure(&d3d9_config);

    hook_d3d9_init(ddrhookx_d3d9_handlers, lengthof(ddrhookx_d3d9_handlers));
}

static DWORD STDCALL
my_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    bool ok;
    struct cconfig *config;

    struct ddrhookx_config_ddrhookx config_ddrhookx;
    struct ddrhookx_config_eamuse config_eamuse;
    struct ddrhookx_config_gfx config_gfx;
    struct ddrhookx_config_security config_security;

    if (ddrhookx_init_check)
        goto skip;

    log_info("--- Begin ddrhookx GetModuleFileNameA ---");

    ddrhookx_init_check = true;

    config = cconfig_init();

    ddrhookx_config_ddrhookx_init(config);
    ddrhookx_config_eamuse_init(config);
    ddrhookx_config_gfx_init(config);
    ddrhookx_config_security_init(config);

    if (!cconfig_hook_config_init(
            config,
            DDRHOOKX_INFO_HEADER "\n" DDRHOOKX_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    ddrhookx_config_ddrhookx_get(&config_ddrhookx, config);
    ddrhookx_config_eamuse_get(&config_eamuse, config);
    ddrhookx_config_gfx_get(&config_gfx, config);
    ddrhookx_config_security_get(&config_security, config);

    cconfig_finit(config);

    standard_def = config_ddrhookx.standard_def;

    log_info(DDRHOOKX_INFO_HEADER);
    log_info("Initializing ddrhookx...");

    ddrhookx_avs_boot_init();
    ddrhookx_avs_boot_set_eamuse_addr(&config_eamuse.server);

    iohook_push_handler(p3io_emu_dispatch_irp);
    iohook_push_handler(extio_dispatch_irp);
    iohook_push_handler(spike_dispatch_irp);
    iohook_push_handler(usbmem_dispatch_irp);

    if (config_ddrhookx.use_com4_emu) {
        /* See ddrhook/p3io.c for details. */
        iohook_push_handler(com4_dispatch_irp);
    }

    ddrhookx_setup_d3d9_hooks(&config_gfx);

    rs232_hook_init();

    p3io_ddr_init_with_plugs(
        &config_security.mcode,
        &config_eamuse.pcbid,
        &config_eamuse.eamid,
        &security_rp_sign_key_black_ddrx,
        &security_rp_sign_key_white_eamuse);
    extio_init();
    usbmem_init(config_ddrhookx.usbmem_path);
    spike_init();
    com4_init();

    log_info("Initializing DDR IO backend");

    ok = ddr_io_init(thread_create, thread_join, thread_destroy);

    if (!ok) {
        log_fatal("Couldn't initialize DDR IO backend");
        return false;
    }

    if (config_ddrhookx.use_com4_emu) {
        log_info("Initializing card reader backend");

        eam_io_set_loggers(
            log_body_misc, log_body_info, log_body_warning, log_body_fatal);

        ok = eam_io_init(thread_create, thread_join, thread_destroy);

        if (!ok) {
            log_fatal("Couldn't initialize card reader backend");
            return false;
        }
    }

    log_info("--- End ddrhookx GetModuleFileNameA ---");

skip:
    return real_GetModuleFileNameA(hModule, lpFilename, nSize);
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        log_to_writer(log_writer_debug, NULL);

        hook_table_apply(
            NULL, "kernel32.dll", init_hook_syms, lengthof(init_hook_syms));

        master_insert_hooks(NULL);
        filesystem_hook_init();

        ddrhookx_d3d9_hook_init();
    }

    return TRUE;
}

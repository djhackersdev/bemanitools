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

#include "ddrhook1/avs-boot.h"
#include "ddrhook1/config-ddrhook1.h"
#include "ddrhook1/config-eamuse.h"
#include "ddrhook1/config-gfx.h"
#include "ddrhook1/config-security.h"
#include "ddrhook1/filesystem.h"
#include "ddrhook1/master.h"

#include "ddrhook-util/gfx.h"

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

#define DDRHOOK1_INFO_HEADER \
    "ddrhook1 for DDR X"    \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define DDRHOOK1_CMD_USAGE \
    "Usage: inject.exe ddrhook1.dll <ddr.exe> [options...]"

bool standard_def;
bool _15khz;

static const hook_d3d9_irp_handler_t ddrhook1_d3d9_handlers[] = {
    gfx_d3d9_irp_handler,
};

static DWORD STDCALL my_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
static DWORD(STDCALL *real_GetModuleFileNameA)(HMODULE hModule, LPSTR lpFilename, DWORD nSize);

static bool ddrhook1_init_check = false;

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "GetModuleFileNameA",
        .patch = my_GetModuleFileNameA,
        .link = (void **) &real_GetModuleFileNameA,
    },
};

static DWORD STDCALL
my_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    bool ok;
    struct cconfig *config;

    struct ddrhook1_config_ddrhook1 config_ddrhook1;
    struct ddrhook1_config_eamuse config_eamuse;
    struct ddrhook1_config_gfx config_gfx;
    struct ddrhook1_config_security config_security;

    if (ddrhook1_init_check)
        goto skip;

    log_info("--- Begin ddrhook1 main ---");

    ddrhook1_init_check = true;

    config = cconfig_init();

    ddrhook1_config_ddrhook1_init(config);
    ddrhook1_config_eamuse_init(config);
    ddrhook1_config_gfx_init(config);
    ddrhook1_config_security_init(config);

    if (!cconfig_hook_config_init(
            config,
            DDRHOOK1_INFO_HEADER "\n" DDRHOOK1_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    ddrhook1_config_ddrhook1_get(&config_ddrhook1, config);
    ddrhook1_config_eamuse_get(&config_eamuse, config);
    ddrhook1_config_gfx_get(&config_gfx, config);
    ddrhook1_config_security_get(&config_security, config);

    cconfig_finit(config);

    standard_def = config_ddrhook1.standard_def;
    _15khz = config_ddrhook1.use_15khz;

    log_info(DDRHOOK1_INFO_HEADER);
    log_info("Initializing ddrhook1...");

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
    usbmem_init(config_ddrhook1.usbmem_path_p1, config_ddrhook1.usbmem_path_p2,
        config_ddrhook1.usbmem_enabled);
    spike_init();
    com4_init();

    log_info("Initializing DDR IO backend");

    ok = ddr_io_init(thread_create, thread_join, thread_destroy);

    if (!ok) {
        log_fatal("Couldn't initialize DDR IO backend");
        return false;
    }

    if (config_ddrhook1.use_com4_emu) {
        log_info("Initializing card reader backend");

        eam_io_set_loggers(
            log_body_misc, log_body_info, log_body_warning, log_body_fatal);

        ok = eam_io_init(thread_create, thread_join, thread_destroy);

        if (!ok) {
            log_fatal("Couldn't initialize card reader backend");
            return false;
        }
    }

    log_info("--- End ddrhook1 main ---");

skip:
    return real_GetModuleFileNameA(hModule, lpFilename, nSize);
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        log_to_writer(log_writer_debug, NULL);

        hook_table_apply(
            NULL, "kernel32.dll", init_hook_syms, lengthof(init_hook_syms));

        ddrhook1_master_insert_hooks(NULL);
        ddrhook1_filesystem_hook_init();

        hook_d3d9_init(ddrhook1_d3d9_handlers, lengthof(ddrhook1_d3d9_handlers));
    }

    return TRUE;
}

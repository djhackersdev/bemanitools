#include <windows.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "ezusb-iidx-emu/nodes.h"

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "ezusb2-iidx-emu/msg.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "iidxhook-util/acio.h"
#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/log-server.h"
#include "iidxhook-util/settings.h"

#include "imports/avs.h"

#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"

#define IIDXHOOK5_INFO_HEADER \
    "iidxhook for Lincle" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK5_CMD_USAGE \
    "Usage: launcher.exe -K iidxhook5.dll <bm2dx.dll> [options...]"

static const irp_handler_t iidxhook_handlers[] = {
    ezusb2_emu_device_dispatch_irp,
    iidxhook_util_acio_dispatch_irp,
    settings_hook_dispatch_irp,
};

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    struct cconfig* config;
    
    struct iidxhook_config_gfx config_gfx;

    log_server_init();
    log_info("-------------------------------------------------------------");
    log_info("--------------- Begin iidxhook dll_entry_init ---------------");
    log_info("-------------------------------------------------------------");

    config = cconfig_init();

    iidxhook_config_gfx_init(config);

    if (!cconfig_hook_config_init(config, IIDXHOOK5_INFO_HEADER "\n" IIDXHOOK5_CMD_USAGE, CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        log_server_fini();
        exit(EXIT_FAILURE);
    }

    iidxhook_config_gfx_get(&config_gfx, config);

    cconfig_finit(config);

    log_info(IIDXHOOK5_INFO_HEADER);
    log_info("Initializing iidxhook...");

    if (config_gfx.windowed) {
        d3d9_set_windowed(config_gfx.framed, config_gfx.window_width,
            config_gfx.window_height);
    }

    if (config_gfx.pci_id_pid != 0 && config_gfx.pci_id_vid != 0) {
        d3d9_set_pci_id(config_gfx.pci_id_pid, config_gfx.pci_id_vid);
    }

    if (config_gfx.frame_rate_limit > 0) {
        d3d9_set_frame_rate_limit(config_gfx.frame_rate_limit);
    }

    if (config_gfx.scale_back_buffer_width > 0 && config_gfx.scale_back_buffer_height > 0) {
        d3d9_scale_back_buffer(config_gfx.scale_back_buffer_width, config_gfx.scale_back_buffer_height,
            config_gfx.scale_back_buffer_filter);
    }

    /* Start up IIDXIO.DLL */
    log_info("Starting IIDX IO backend");
    iidx_io_set_loggers(log_impl_misc, log_impl_info, log_impl_warning,
            log_impl_fatal);

    if (!iidx_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy)) {
        log_fatal("Initializing IIDX IO backend failed");
    }

    /* Start up EAMIO.DLL */
    log_misc("Initializing card reader backend");
    eam_io_set_loggers(log_impl_misc, log_impl_info, log_impl_warning,
            log_impl_fatal);

    if (!eam_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy)) {
        log_fatal("Initializing card reader backend failed");
    }

    /* Set up IO emulation hooks _after_ IO API setup to allow
       API implementations with real IO devices */
    iohook_init(iidxhook_handlers, lengthof(iidxhook_handlers));

    hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
    ezusb2_emu_device_hook_init(ezusb2_iidx_emu_msg_init());

    /* Card reader emulation, same issue with hooking as IO emulation */
    rs232_hook_init();

    /* Do not use legacy mode, first version with wave pass readers */
    iidxhook_util_acio_init(false);

    log_info("-------------------------------------------------------------");
    log_info("---------------- End iidxhook dll_entry_init ----------------");
    log_info("-------------------------------------------------------------");

    return app_hook_invoke_init(sidcode, param);
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    log_misc("Shutting down card reader backend");
    eam_io_fini();

    log_misc("Shutting down IIDX IO backend");
    iidx_io_fini();

    log_server_fini();

    return result;
}

/**
 * Hook library for Lincle
 */
BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    log_to_external(
            log_body_misc,
            log_body_info,
            log_body_warning,
            log_body_fatal);

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    acp_hook_init();
    adapter_hook_init();
    d3d9_hook_init();
    settings_hook_init();

end:
    return TRUE;
}


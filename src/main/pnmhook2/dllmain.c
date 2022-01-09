#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/iidxio.h"

#include "cconfig/cconfig-hook.h"

#include "hooklib/acp.h"
#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "imports/avs.h"

#include "iidxhook-util/config-gfx.h"
#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/log-server.h"
#include "iidxhook-util/settings.h"

#include "pnmhook2/acio.h"

#include "util/log.h"
#include "util/str.h"
#include "util/thread.h"

#define IIDXHOOK7_INFO_HEADER                         \
    "iidxhook for SPADA, PENDUAL, copula and SINOBUZ" \
    ", build " __DATE__ " " __TIME__ ", gitrev " STRINGIFY(GITREV)
#define IIDXHOOK7_CMD_USAGE \
    "Usage: launcher.exe -K iidxhook7.dll <bm2dx.dll> [options...]"

static const hook_d3d9_irp_handler_t iidxhook_d3d9_handlers[] = {
    iidxhook_util_d3d9_irp_handler,
};

static void
iidxhook7_setup_d3d9_hooks(const struct iidxhook_config_gfx *config_gfx)
{
    struct iidxhook_util_d3d9_config d3d9_config;

    iidxhook_util_d3d9_init_config(&d3d9_config);

    // TODO hardcoded because popn crashes otherwise in vm due to not matching to its native res
    d3d9_config.windowed = true;
    d3d9_config.framed = true;
    d3d9_config.override_window_width = 1360;
    d3d9_config.override_window_height = 768;
    d3d9_config.framerate_limit = config_gfx->frame_rate_limit;
    d3d9_config.pci_vid = config_gfx->pci_id_vid;
    d3d9_config.pci_pid = config_gfx->pci_id_pid;
    d3d9_config.scale_back_buffer_width = 1360;
    d3d9_config.scale_back_buffer_height = 768;
    d3d9_config.scale_back_buffer_filter = config_gfx->scale_back_buffer_filter;
    d3d9_config.forced_refresh_rate = config_gfx->forced_refresh_rate;
    d3d9_config.device_adapter = config_gfx->device_adapter;

    iidxhook_util_d3d9_configure(&d3d9_config);

    hook_d3d9_init(iidxhook_d3d9_handlers, lengthof(iidxhook_d3d9_handlers));
}

// TODO move to own module
static void ezusb_proxy_boot() 
{
    
}

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    struct cconfig *config;

    struct iidxhook_config_gfx config_gfx;

    // TODO re-enable once done debugging
    // TODO note: i have a weird feeling about our log server implemetnation being a source for
    // several random errors and maybe even performacne issues. removing this and pnmhook2
    // stopped hanging in the weirdest places that don't make sense
    //log_server_init();
    log_to_writer(log_writer_stdout, NULL);

    log_info("-------------------------------------------------------------");
    log_info("--------------- Begin pnmhook dll_entry_init ---------------");
    log_info("-------------------------------------------------------------");

    acp_hook_init();
    adapter_hook_init();

    // using settings_hook_init is good enough here, copy paste the code later in its own pnm utils
    // module
    // TODO e/up needs to be manually deleted after each boot, otherwise the game gets stuck during boot
   
   
    settings_hook_init();
   
    //iohook_push_handler(settings_hook_dispatch_irp);

    log_info("asdf1111");
    log_info(">>>>>");

    config = cconfig_init();
    log_info(">>>>>");
    iidxhook_config_gfx_init(config);
    log_info(">>>>>");
    if (!cconfig_hook_config_init(
            config,
            IIDXHOOK7_INFO_HEADER "\n" IIDXHOOK7_CMD_USAGE,
            CCONFIG_CMD_USAGE_OUT_DBG)) {
        cconfig_finit(config);
        log_server_fini();
        exit(EXIT_FAILURE);
    }

    log_info(">>>>>");
    iidxhook_config_gfx_get(&config_gfx, config);
    log_info(">>>>>");
    cconfig_finit(config);
    log_info(">>>>>");
    iidxhook7_setup_d3d9_hooks(&config_gfx);
    log_info(">>>>>");
    /* Start up EAMIO.DLL */
    log_misc("Initializing card reader backend");

    eam_io_set_loggers(
        log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);
log_info(">>>> 222223333");
    // TODO gets stuck
    if (!eam_io_init(thread_create, thread_join, thread_destroy)) {
        log_fatal("Initializing card reader backend failed");
    }
log_info(">>>>");

    iohook_push_handler(ac_io_port_dispatch_irp);

log_info(">>>>333");
   /* Card reader emulation, same issue with hooking as IO emulation */
    rs232_hook_init();
log_info(">>>>");

    ac_io_port_init();
log_info(">>>>");
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

    log_server_fini();

    return result;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        goto end;
    }

    log_to_external(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    log_info(">>>>>>>");

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

end:
    return TRUE;
}

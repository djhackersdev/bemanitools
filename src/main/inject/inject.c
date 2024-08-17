#define LOG_MODULE "inject"

#include "iface-core/log.h"

#include "inject/debugger.h"
#include "inject/inject-config.h"
#include "inject/logger.h"
#include "inject/version.h"

#include "util/os.h"
#include "util/signal.h"

static void _inject_fini();

static void _inject_signal_handler_shutdown()
{
    _inject_fini();
    exit(0);
}

static void _inject_header_log()
{
    log_info(
        "\n"
        "  _        _           _   \n"
        " (_)_ __  (_) ___  ___| |_ \n"
        " | | '_ \\ | |/ _ \\/ __| __|\n"
        " | | | | || |  __/ (__| |_ \n"
        " |_|_| |_|/ |\\___|\\___|\\__|\n"
        "        |__/               ");

    log_info(
        "build date %s, gitrev %s", inject_build_date, inject_gitrev);
}

static void _inject_iat_hook_dlls(const struct hooks_iat_config *configs)
{
    uint32_t i;

    log_assert(configs);

    log_info("Injecting IAT hook DLLs...");

    for (i = 0; i < HOOKS_CONFIG_MAX_HOOKS; i++) {
        if (hooks_config_iat_is_valid(&configs[i])) {
            if (configs[i].enable) {
                if (!debugger_replace_dll_iat(configs[i].source_name, configs[i].path)) {
                    log_fatal("Injecting iat hook failed: %s=%s", configs[i].source_name, configs[i].path);
                }
            } else {
                log_warning("iat hook disabled: %s=%s", configs[i].source_name, configs[i].path);
            }
        }
    }
}

static void _inject_hook_dlls(const struct hooks_hook_config *configs)
{
    uint32_t i;

    log_assert(configs);

    log_info("Injecting hook DLLs...");

    for (i = 0; i < HOOKS_CONFIG_MAX_HOOKS; i++) {
        if (hooks_config_hook_is_valid(&configs[i])) {
            if (configs[i].enable) {
                if (!debugger_inject_dll(configs[i].path)) {
                    log_fatal("Injecting hook failed: %s", configs[i].path);
                }
            } else {
                log_warning("Hook disabled: %s", configs[i].path);
            }
        }
    }
}

static void _inject_init(const inject_config_t *config)
{
    log_assert(config);

    logger_init(&config->logger);

    _inject_header_log();
    os_version_log();

    signal_exception_handler_init();
    // Cleanup remote process on CTRL+C
    signal_register_shutdown_handler(_inject_signal_handler_shutdown);

    debugger_init(
        config->debugger.attach_type,
        config->debugger.app.path,
        config->debugger.app.args);

    log_misc("<<< init");
}

static void _inject_run(const inject_config_t *config)
{
    log_assert(config);

    log_misc(">>> run");
    
    _inject_hook_dlls(config->hooks.hooks);
    _inject_iat_hook_dlls(config->hooks.iats);

    debugger_run();

    log_misc("<<< run");
}

static void _inject_fini()
{
    log_misc(">>> fini");

    debugger_finit(false);

    logger_fini();
}

// TODO run inject module only with inject configuration
// config must be bootstrapped using an early env 
// in main with a early logger setup etc.
// apply the same to launcher
void inject_main(const inject_config_t *config)
{
    log_assert(config);

    log_misc(">>> main");

    _inject_init(config);
    _inject_run(config);
    _inject_fini();
}
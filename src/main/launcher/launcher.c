#define LOG_MODULE "launcher"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-file.h"
#include "core/log-sink-list.h"
#include "core/log-sink-std.h"
#include "core/log.h"

#include "imports/avs-ea3.h"
#include "imports/avs.h"

#include "launcher/avs-config.h"
#include "launcher/avs.h"
#include "launcher/bootstrap-config.h"
#include "launcher/bootstrap.h"
#include "launcher/debug.h"
#include "launcher/ea3-ident-config.h"
#include "launcher/eamuse-config.h"
#include "launcher/eamuse.h"
#include "launcher/hook.h"
#include "launcher/launcher-config.h"
#include "launcher/module.h"
#include "launcher/options.h"
#include "launcher/property-util.h"
#include "launcher/stubs.h"
#include "launcher/version.h"

#include "procmon-lib/procmon.h"

#include "util/debug.h"
#include "util/defs.h"
#include "util/fs.h"
#include "util/os.h"
#include "util/proc.h"
#include "util/signal.h"
#include "util/str.h"

static void _launcher_log_header()
{
    log_info(
        "\n"
        " .__                             .__                   \n"
        " |  | _____   __ __  ____   ____ |  |__   ___________  \n"
        " |  | \\__  \\ |  |  \\/    \\_/ ___\\|  |  \\_/ __ \\_  __ \\ \n"
        " |  |__/ __ \\|  |  /   |  \\  \\___|   Y  \\  ___/|  | \\/ \n"
        " |____(____  /____/|___|  /\\___  >___|  /\\___  >__|    \n"
        "          \\/           \\/     \\/     \\/     \\/       ");

    log_info(
        "launcher build date %s, gitrev %s",
        launcher_build_date,
        launcher_gitrev);
}

void _launcher_log_init(
    const char *log_file_path, enum core_log_bt_log_level level)
{
    struct core_log_sink sinks[2];
    struct core_log_sink sink_composed;

    core_log_bt_ext_impl_set();

    if (log_file_path) {
        core_log_sink_std_out_open(true, &sinks[0]);
        core_log_sink_file_open(log_file_path, false, true, 10, &sinks[1]);
        core_log_sink_list_open(sinks, 2, &sink_composed);
    } else {
        core_log_sink_std_out_open(true, &sink_composed);
    }

    core_log_bt_init(&sink_composed);
    core_log_bt_level_set(level);
}

static void _launcher_signal_shutdown_handler()
{
    core_log_bt_fini();
    ExitProcess(EXIT_FAILURE);
}

static void _launcher_env_game_dir_verify()
{
    char cwd[MAX_PATH];
    char modules_dir[MAX_PATH];
    char prop_dir[MAX_PATH];

    getcwd(cwd, sizeof(cwd));

    log_info("Current working directory: %s", cwd);

    str_cpy(modules_dir, sizeof(modules_dir), cwd);
    str_cpy(prop_dir, sizeof(prop_dir), cwd);

    str_cat(modules_dir, sizeof(modules_dir), "/modules");
    str_cat(prop_dir, sizeof(prop_dir), "/prop");

    if (!path_exists(modules_dir)) {
        log_fatal(
            "Cannot find 'modules' directory in current working directory: %s",
            cwd);
    }

    if (!path_exists(prop_dir)) {
        log_fatal(
            "Cannot find 'prop' directory in current working directory: %s",
            cwd);
    }
}

static void _launcher_bootstrap_config_options_override(
    struct launcher_bootstrap_config *config,
    const struct options_bootstrap *options)
{
    log_assert(config);
    log_assert(options);

    if (options->config_path) {
        log_misc(
            "Command line override bootstrap configuration from file: %s",
            options->config_path);

        property_util_free(config->property);
        config->property = property_util_load(options->config_path);
    }

    if (options->selector) {
        log_misc(
            "Command line override bootstrap selector: %s", options->selector);

        str_cpy(config->selector, sizeof(config->selector), options->selector);
    }
}

static void _launcher_ea3_ident_config_options_override(
    struct ea3_ident_config *config, const struct options_eamuse *options)
{
    log_assert(config);
    log_assert(options);

    if (options->softid) {
        str_cpy(config->softid, sizeof(config->softid), options->softid);
    }

    if (options->pcbid) {
        str_cpy(config->pcbid, sizeof(config->pcbid), options->pcbid);
    }
}

static void _launcher_hook_config_options_override(
    struct launcher_config *config, const struct options_hook *options)
{
    size_t i;
    const char *dll;

    log_assert(config);
    log_assert(options);

    for (i = 0; i < options->hook_dlls.nitems; i++) {
        dll = *array_item(const char *, &options->hook_dlls, i);

        if (!launcher_config_add_hook_dll(config, dll)) {
            log_warning(
                "Adding override hook dll '%s' failed (max supported limit "
                "exceeded), ignored",
                dll);
        }
    }

    for (i = 0; i < options->before_hook_dlls.nitems; i++) {
        dll = *array_item(const char *, &options->before_hook_dlls, i);

        if (!launcher_config_add_before_hook_dll(config, dll)) {
            log_warning(
                "Adding override before hook dll '%s' failed (max supported "
                "limit exceeded), ignored",
                dll);
        }
    }

    for (i = 0; i < options->iat_hook_dlls.nitems; i++) {
        dll = *array_item(const char *, &options->iat_hook_dlls, i);

        if (!launcher_config_add_iat_hook_dll(config, dll)) {
            log_warning(
                "Adding override iat hook dll '%s' failed (max supported limit "
                "exceeded), ignored",
                dll);
        }
    }
}

static void _launcher_debug_config_options_override(
    struct launcher_debug_config *config, const struct options_debug *options)
{
    log_assert(config);
    log_assert(options);

    if (options->remote_debugger) {
        log_misc("Command line override, enable remote debugger");

        config->remote_debugger = true;
    }

    if (options->log_property_configs) {
        log_misc("Command line override, log property configs");

        config->log_property_configs = true;
    }
}

static void _launcher_config_options_override(
    struct launcher_config *config, const struct options *options)
{
    log_assert(config);
    log_assert(options);

    // Apply command line overrides on all launcher owned configuration
    // parameters
    _launcher_bootstrap_config_options_override(
        &config->bootstrap, &options->bootstrap);
    _launcher_hook_config_options_override(config, &options->hook);
    _launcher_debug_config_options_override(&config->debug, &options->debug);
}

static void
_launcher_config_full_resolved_log(const struct launcher_config *config)
{
    if (config->debug.log_property_configs) {
        log_misc("launcher-config resolved properties");
        log_misc("bootstrap-config");
        property_util_log(config->bootstrap.property);

        log_misc("avs-config");
        property_util_log(config->avs.property);

        log_misc("ea3-ident-config");
        property_util_log(config->ea3_ident.property);

        log_misc("eamuse-config");
        property_util_log(config->eamuse.property);
    }
}

static void
_launcher_remote_debugger_trap(const struct launcher_debug_config *config)
{
    log_assert(config);

    /* If enabled, wait for a remote debugger to attach as early as possible.
       Spawning launcher with a debugger crashes it for some reason
       (e.g. on jubeat08). However, starting the launcher separately and
       attaching a remote debugger works */

    if (config->remote_debugger) {
        debug_remote_debugger_trap();
    }
}

static void _launcher_procmon_init(
    const struct launcher_debug_config *config,
    struct procmon *procmon)
{
    procmon_init(procmon);

    if (procmon_available()) {
        procmon_load(procmon);

        core_log_impl_assign(procmon->set_loggers);
        procmon->init();

        if (config->procmon_file) {
            procmon->file_mon_enable();
        }

        if (config->procmon_module) {
            procmon->module_mon_enable();
        }

        if (config->procmon_thread) {
            procmon->thread_mon_enable();
        }
    }
}

static void _launcher_bootstrap_config_load(
    const struct launcher_bootstrap_config *launcher_bootstrap_config,
    struct bootstrap_config *config)
{
    bootstrap_config_init(config);

    bootstrap_config_load(
        launcher_bootstrap_config->property,
        launcher_bootstrap_config->selector,
        config);
}

static void _launcher_bootstrap_log_config_options_override(
    struct bootstrap_log_config *config, const struct options_log *options)
{
    log_assert(config);
    log_assert(options);

    if (options->level) {
        log_misc(
            "Command line override bootstrap log level: %d", *(options->level));

        switch (*(options->level)) {
            case CORE_LOG_BT_LOG_LEVEL_OFF:
                str_cpy(config->level, sizeof(config->level), "disable");
                break;

            case CORE_LOG_BT_LOG_LEVEL_FATAL:
                str_cpy(config->level, sizeof(config->level), "fatal");
                break;

            case CORE_LOG_BT_LOG_LEVEL_WARNING:
                str_cpy(config->level, sizeof(config->level), "warn");
                break;

            case CORE_LOG_BT_LOG_LEVEL_INFO:
                str_cpy(config->level, sizeof(config->level), "info");
                break;

            case CORE_LOG_BT_LOG_LEVEL_MISC:
                str_cpy(config->level, sizeof(config->level), "misc");
                break;

            default:
                log_assert(false);
        }
    }

    if (options->file_path) {
        log_misc(
            "Command line override bootstrap log file: %s", options->file_path);
        str_cpy(config->file, sizeof(config->file), options->file_path);
    }
}

static void _launcher_bootstrap_log_config_verify(
    const struct launcher_config *launcher_config,
    const struct bootstrap_config *bootstrap_config)
{
    log_assert(launcher_config);
    log_assert(bootstrap_config);

    if (!str_eq(bootstrap_config->startup.log.level, "misc")) {
        if (launcher_config->debug.log_property_configs) {
            log_warning(
                "Logging of property configs enabled, but requires misc log "
                "level, current log level: %s",
                bootstrap_config->startup.log.level);
        }
    }
}

static void
_launcher_before_hook_dlls_load(const struct launcher_hook_config *config)
{
    int i;

    log_assert(config);

    log_misc("Loading before hook dlls...");

    for (i = 0; i < LAUNCHER_CONFIG_MAX_HOOK_DLL; i++) {
        if (strlen(config->before_hook_dlls[i]) > 0) {
            hook_load_dll(config->before_hook_dlls[i]);
        }
    }
}

static void _launcher_ea3_ident_config_load(
    const struct launcher_ea3_ident_config *launcher_config,
    struct ea3_ident_config *config,
    bool log_property_configs)
{
    log_assert(launcher_config);
    log_assert(config);

    ea3_ident_config_init(config);
    ea3_ident_config_load(launcher_config->property, config);

    if (log_property_configs) {
        log_misc("Property ea3-ident-config");

        property_util_log(launcher_config->property);
    }

    if (!ea3_ident_config_hardid_is_defined(config)) {
        log_misc(
            "No no hardid defined in ea3-ident-config, derive from ethernet");

        ea3_ident_config_hardid_from_ethernet_set(config);
    }
}

static void _launcher_bootstrap_module_init(
    const struct bootstrap_module_config *module_config,
    const struct launcher_hook_config *hook_config)
{
    int i;
    struct array iat_hook_dlls;

    log_assert(module_config);
    log_assert(hook_config);

    array_init(&iat_hook_dlls);

    for (i = 0; i < LAUNCHER_CONFIG_MAX_HOOK_DLL; i++) {
        if (strlen(hook_config->before_hook_dlls[i]) > 0) {
            *array_append(const char *, &iat_hook_dlls) =
                (const char *) &hook_config->before_hook_dlls[i];
        }
    }

    bootstrap_module_init(module_config, &iat_hook_dlls);

    array_fini(&iat_hook_dlls);
}

static void _launcher_hook_dlls_load(const struct launcher_hook_config *config)
{
    int i;

    log_assert(config);

    log_misc("Loading hook dlls...");

    for (i = 0; i < LAUNCHER_CONFIG_MAX_HOOK_DLL; i++) {
        if (strlen(config->hook_dlls[i]) > 0) {
            hook_load_dll(config->hook_dlls[i]);
        }
    }
}

static void _launcher_dongle_stubs_init()
{
    stubs_init();
}

static void _launcher_debugger_break()
{
    /* Opportunity for breakpoint setup etc */
    if (IsDebuggerPresent()) {
        DebugBreak();
    }
}

void _launcher_log_reinit()
{
    core_log_bt_ext_impl_set();
}

void _launcher_init(
    const struct options *options,
    struct launcher_config *launcher_config,
    struct bootstrap_config *bootstrap_config,
    struct ea3_ident_config *ea3_ident_config,
    struct procmon *procmon)
{
    struct property *launcher_property;

    log_assert(options);
    log_assert(launcher_config);
    log_assert(bootstrap_config);
    log_assert(ea3_ident_config);

    // Early logging pre AVS setup depend entirely on command args
    // We don't even have the bootstrap configuration loaded at this point
    _launcher_log_init(options->log.file_path, *(options->log.level));
    _launcher_log_header();

    debug_init(core_log_fatal_impl_get());
    signal_exception_handler_init(_launcher_signal_shutdown_handler);
    signal_register_shutdown_handler(&_launcher_signal_shutdown_handler);

    os_version_log();
    _launcher_env_game_dir_verify();

    if (proc_is_running_as_admin_user()) {
        log_warning(
            "Not running as admin user. Launcher and games require elevated "
            "privileges to run correctly");
    }

    launcher_config_init(launcher_config);

    if (options->launcher.config_path) {
        log_info(
            "Loading launcher configuration from file: %s",
            options->launcher.config_path);

        launcher_property = property_util_load(options->launcher.config_path);
        launcher_config_load(launcher_property, launcher_config);

        _launcher_config_options_override(launcher_config, options);

        if (launcher_config->debug.log_property_configs) {
            log_misc("launcher-config");
            property_util_log(launcher_property);
        }

        property_util_free(launcher_property);
    } else {
        _launcher_config_options_override(launcher_config, options);
    }

    // Not really fully resolved, but have an early debug dump because there are
    // still several more steps that can fail before having the entire
    // configuration resolved
    _launcher_config_full_resolved_log(launcher_config);

    _launcher_remote_debugger_trap(&launcher_config->debug);

    _launcher_procmon_init(&launcher_config->debug, procmon);

    _launcher_bootstrap_config_load(
        &launcher_config->bootstrap, bootstrap_config);
    _launcher_bootstrap_log_config_options_override(
        &bootstrap_config->startup.log, &options->log);
    _launcher_bootstrap_log_config_verify(launcher_config, bootstrap_config);

    bootstrap_init(launcher_config->debug.log_property_configs);
    bootstrap_log_init(&bootstrap_config->startup.log);

    _launcher_before_hook_dlls_load(&launcher_config->hook);

    bootstrap_avs_init(
        &bootstrap_config->startup.boot,
        &bootstrap_config->startup.log,
        avs_config_root_get(launcher_config->avs.property));
    bootstrap_default_files_create(&bootstrap_config->startup.default_file);

    _launcher_ea3_ident_config_load(
        &launcher_config->ea3_ident,
        ea3_ident_config,
        launcher_config->debug.log_property_configs);
    _launcher_ea3_ident_config_options_override(
        ea3_ident_config, &options->eamuse);

    // Execute another one which is now actually final. No more configuration
    // changes from this point on
    _launcher_config_full_resolved_log(launcher_config);
}

void _launcher_run(
    const struct launcher_config *launcher_config,
    const struct bootstrap_config *bootstrap_config,
    struct ea3_ident_config *ea3_ident_config)
{
    log_assert(launcher_config);
    log_assert(bootstrap_config);
    log_assert(ea3_ident_config);

    _launcher_bootstrap_module_init(
        &bootstrap_config->startup.module, &launcher_config->hook);

    _launcher_hook_dlls_load(&launcher_config->hook);

    _launcher_dongle_stubs_init();

    _launcher_debugger_break();

    bootstrap_module_game_init(
        &bootstrap_config->startup.module, ea3_ident_config);

    bootstrap_eamuse_init(
        &bootstrap_config->startup.eamuse,
        ea3_ident_config,
        eamuse_config_root_get(launcher_config->eamuse.property));

    bootstrap_module_game_run();
}

void _launcher_fini(
    struct launcher_config *launcher_config,
    const struct bootstrap_config *bootstrap_config,
    struct procmon *procmon)
{
    log_assert(launcher_config);
    log_assert(bootstrap_config);
    log_assert(procmon);

    bootstrap_eamuse_fini(&bootstrap_config->startup.eamuse);

    bootstrap_avs_fini();

    _launcher_log_reinit();

    bootstrap_module_game_fini();

    if (procmon->module != NULL) {
        procmon_free(procmon);
    }

    launcher_config_fini(launcher_config);

    log_info("Shutdown complete");

    core_log_bt_fini();
}

void launcher_main(const struct options *options)
{
    struct launcher_config launcher_config;
    struct bootstrap_config bootstrap_config;
    struct ea3_ident_config ea3_ident_config;
    struct procmon procmon;

    log_assert(options);

    _launcher_init(
        options, &launcher_config, &bootstrap_config, &ea3_ident_config, &procmon);

    _launcher_run(&launcher_config, &bootstrap_config, &ea3_ident_config);

    _launcher_fini(&launcher_config, &bootstrap_config, &procmon);
}
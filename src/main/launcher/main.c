#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs-ea3.h"
#include "imports/avs.h"

#include "launcher/avs-context.h"
#include "launcher/bootstrap-config.h"
#include "launcher/bootstrap-context.h"
#include "launcher/ea3-ident.h"
#include "launcher/eamuse.h"
#include "launcher/logger.h"
#include "launcher/module.h"
#include "launcher/options.h"
#include "launcher/property.h"
#include "launcher/stubs.h"

#include "util/defs.h"
#include "util/fs.h"
#include "util/log.h"
#include "util/os.h"
#include "util/str.h"

static void log_avs_fs_dir(const char *path)
{
    const char* name;

    avs_desc dir = avs_fs_opendir(path);

    if (dir < 0) {
        log_warning("Opening avs dir %s failed, skipping logging contents", path);
    }

    log_misc("Contents of %s:", path);

    do {
        name = avs_fs_readdir(dir);

        if (name == NULL) {
            break;
        }

        log_misc("%s", name);
    } while (name != NULL);

    avs_fs_closedir(dir);
}

static void log_env_info()
{
    char buffer_tmp[MAX_PATH];

    os_version_log();
    
    getcwd(buffer_tmp, sizeof(buffer_tmp));
    log_info("Current working directory: %s", buffer_tmp);
}

static void trap_remote_debugger()
{
    BOOL res;

    log_info("Waiting until debugger attaches to remote process...");

    while (true) {
        res = FALSE;
        
        if (!CheckRemoteDebuggerPresent(GetCurrentProcess(), &res)) {
            log_fatal(
                "CheckRemoteDebuggerPresent failed: %08x",
                (unsigned int) GetLastError());
        }

        if (res) {
            log_info("Debugger attached, resuming");
            break;
        }

        Sleep(1000);
    }
}

static void avs_config_setup(
        struct bootstrap_config *bootstrap_config,
        const char* dev_nvram_raw_path,
        bool override_loglevel_enabled,
        enum log_level loglevel,
        struct property **avs_config_property)
{
    struct property_node *avs_config_node;

    log_assert(bootstrap_config);
    log_assert(avs_config_property);

    *avs_config_property = boot_property_load(bootstrap_config->startup.avs.config_file);
    avs_config_node = property_search(*avs_config_property, 0, "/config");

    if (avs_config_node == NULL) {
        log_fatal("%s: /config missing", bootstrap_config->startup.avs.config_file);
    }

    if (dev_nvram_raw_path) {
        if (!path_exists(dev_nvram_raw_path)) {
            log_warning("Override local file system dev/nvram and dev/raw path %s does not exist, creating",
                dev_nvram_raw_path);

            if (!path_mkdir(dev_nvram_raw_path)) {
                log_fatal("Creating directory %s failed", dev_nvram_raw_path);
            }
        }

        avs_context_property_set_local_fs_nvram_raw(
            *avs_config_property,
            dev_nvram_raw_path);
    }

    if (override_loglevel_enabled) {
        avs_context_property_set_log_level(*avs_config_property, loglevel);
    }

    bootstrap_config_update_avs(bootstrap_config, avs_config_node);
}

static void load_hook_dlls(struct array *hook_dlls)
{
    const char *hook_dll;

    for (size_t i = 0; i < hook_dlls->nitems; i++) {
        hook_dll = *array_item(char *, hook_dlls, i);

        log_info("Load hook dll: %s", hook_dll);

        if (LoadLibraryA(hook_dll) == NULL) {
            LPSTR buffer;

            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR) &buffer,
                0,
                NULL);

            log_fatal("%s: Failed to load hook DLL: %s", hook_dll, buffer);

            LocalFree(buffer);
        }
    }
}

static void ea3_ident_config_setup(
    const char *ea3_ident_path,
    const char *softid,
    const char *pcbid,
    struct ea3_ident *ea3_ident)
{
    log_assert(ea3_ident);

    ea3_ident_init(ea3_ident);

    if (ea3_ident_path) {
        log_misc("Loading ea3-ident from file: %s", ea3_ident_path);
        ea3_ident_initialize_from_file(ea3_ident_path, ea3_ident);
    }

    if (softid) {
        str_cpy(ea3_ident->softid, lengthof(ea3_ident->softid), softid);
    }

    if (pcbid) {
        str_cpy(ea3_ident->pcbid, lengthof(ea3_ident->pcbid), pcbid);
    }

    if (!ea3_ident->hardid[0]) {
        ea3_ident_hardid_from_ethernet(ea3_ident);
    }
}

static void app_config_setup(
    const struct bootstrap_config *bootstrap_config,
    const char *app_config_path,
    struct property **app_config_property,
    struct property_node **app_config_node)
{
    log_assert(bootstrap_config);
    log_assert(app_config_property);
    log_assert(app_config_node);

    if (bootstrap_config->module_params) {
        *app_config_property = NULL;
        *app_config_node = bootstrap_config->module_params;
    } else if (app_config_path && path_exists(app_config_path)) {
        log_misc("Loading avs-config from file: %s", app_config_path);

        *app_config_property = boot_property_load_avs(app_config_path);

        *app_config_node = property_search(*app_config_property, 0, "/param");

        if (app_config_node == NULL) {
            log_fatal("%s: /param missing", app_config_path);
        }
    } else {
        log_warning("Explicit app config (file) missing, defaulting to empty");

        *app_config_property = boot_property_load_cstring("<param>dummy</param>");
        *app_config_node = property_search(*app_config_property, 0, "/param");
    }
}

void invoke_dll_module_init(
    struct ea3_ident *ident,
    const struct module_context *module,
    struct property_node *app_config)
{
    char sidcode_short[17];
    char sidcode_long[21];
    char security_code[9];
    bool ok;

    /* Set up security env vars */

    str_format(
        security_code,
        lengthof(security_code),
        "G*%s%s%s%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev);

    std_setenv("/env/boot/version", "0.0.0");
    std_setenv("/env/profile/security_code", security_code);
    std_setenv("/env/profile/system_id", ident->pcbid);
    std_setenv("/env/profile/account_id", ident->pcbid);
    std_setenv("/env/profile/license_id", ident->softid);
    std_setenv("/env/profile/software_id", ident->softid);
    std_setenv("/env/profile/hardware_id", ident->hardid);

    /* Set up the short sidcode string, let dll_entry_init mangle it */

    str_format(
        sidcode_short,
        lengthof(sidcode_short),
        "%s%s%s%s%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev,
        ident->ext);

    /* Set up long-form sidcode env var */

    str_format(
        sidcode_long,
        lengthof(sidcode_long),
        "%s:%s:%s:%s:%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev,
        ident->ext);

    /* Set this up beforehand, as certain games require it in dll_entry_init */

    std_setenv("/env/profile/soft_id_code", sidcode_long);

    ok = module_context_invoke_init(module, sidcode_short, app_config);

    if (!ok) {
        log_fatal("%s: dll_module_init() returned failure", module->path);
    }

    /* Back-propagate sidcode, as some games modify it during init */

    memcpy(ident->model, sidcode_short + 0, sizeof(ident->model) - 1);
    ident->dest[0] = sidcode_short[3];
    ident->spec[0] = sidcode_short[4];
    ident->rev[0] = sidcode_short[5];
    memcpy(ident->ext, sidcode_short + 6, sizeof(ident->ext));

    /* Set up long-form sidcode env var again */

    str_format(
        sidcode_long,
        lengthof(sidcode_long),
        "%s:%s:%s:%s:%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev,
        ident->ext);

    std_setenv("/env/profile/soft_id_code", sidcode_long);
}

int main(int argc, const char **argv)
{
    struct options options;

    struct property *bootstrap_config_property;
    struct bootstrap_config bootstrap_config;

    struct module_context module;

    struct property *avs_config_property;

    struct ea3_ident ea3_ident;

    struct property *app_config_property;
    struct property_node *app_config_node;
    
    options_init(&options);

    if (!options_read_cmdline(&options, argc, argv) ||
           (!options.bootstrap_selector && !options.module)) {
        options_print_usage();

        return EXIT_FAILURE;
    }

    /* Static logging setup prior AVS available */

    logger_init(options.logfile);

    // Enforce user configured log level
    if (options.override_loglevel_enabled) {
        log_set_level(options.loglevel);
    }

    log_env_info();

    /* If enabled, wait for a remote debugger to attach. Spawning launcher
       with a debugger crashes it for some reason (e.g. on jubeat08). However,
       starting the launcher separately and attaching a remote debugger works */

    if (options.remote_debugger) {
        trap_remote_debugger();
    }

    /* Before hook dlls */

    log_misc("Loading before hook dlls...");
    load_hook_dlls(&options.before_hook_dlls);

    /* Bootstrap either via a bootstrap.xml file or command line arguments */

    if (!options.bootstrap_selector) {
        bootstrap_context_init(
            options.avs_config_path,
            options.ea3_config_path,
            options.std_heap_size,
            options.avs_heap_size,
            options.logfile,
            options.module,
            &bootstrap_config);

        bootstrap_config_property = NULL;
    } else {
        bootstrap_context_init_from_file(
            options.bootstrap_config_path,
            options.bootstrap_selector,
            &bootstrap_config_property,
            &bootstrap_config);

        if (options.log_property_configs) {
            log_misc("Property bootstrap-config");
            boot_property_log(bootstrap_config_property);
        }
    }

    /* AVS */

    avs_config_setup(
        &bootstrap_config,
        options.avs_fs_dev_nvram_raw_path,
        options.override_loglevel_enabled,
        options.loglevel,
        &avs_config_property);

    if (options.log_property_configs) {
        log_misc("Property avs-config");
        boot_property_log(avs_config_property);
    }

    avs_context_init(
        avs_config_property,
        property_search(avs_config_property, 0, "/config"),
        bootstrap_config.startup.avs.avs_heap_size,
        bootstrap_config.startup.avs.std_heap_size);
    
    bootstrap_context_post_avs_setup(&bootstrap_config);
  
    /* Load game DLL */

    log_info("Load game DLL: %s", bootstrap_config.startup.module.file);

    if (options.iat_hook_dlls.nitems > 0) {
        module_context_init_with_iat_hooks(
            &module, bootstrap_config.startup.module.file, &options.iat_hook_dlls);
    } else {
        module_context_init(&module, bootstrap_config.startup.module.file);
    }

    /* Load hook DLLs */

    load_hook_dlls(&options.hook_dlls);

    /* Inject GetModuleHandle hooks */

    stubs_init();

    /* More configuration setup */

    ea3_ident_config_setup(
        options.ea3_ident_path,
        options.softid,
        options.pcbid,
        &ea3_ident);
    
    app_config_setup(
        &bootstrap_config,
        options.app_config_path,
        &app_config_property,
        &app_config_node);

    /* Opportunity for breakpoint setup etc */

    if (IsDebuggerPresent()) {
        DebugBreak();
    }

    /* Initialize of game module */

    if (options.log_property_configs) {
        log_misc("Property app-config");
        boot_property_node_log(app_config_node);
    }

    invoke_dll_module_init(&ea3_ident, &module, app_config_node);

    /* Start up e-Amusement client */

    eamuse_init(
        &bootstrap_config.startup.eamuse,
        &ea3_ident,
        &options);

    /* Run application */

    module_context_invoke_main(&module);

    /* Shut down */

    log_info("Shutting down launcher...");

    eamuse_fini();

    app_config_node = NULL;
    if (app_config_property) {
        boot_property_free(app_config_property);
    }

    log_to_writer(log_writer_file, stdout);
    avs_context_fini();
    boot_property_free(avs_config_property);

    module_context_fini(&module);

    if (bootstrap_config_property) {
        boot_property_free(bootstrap_config_property);
    }

    options_fini(&options);

    log_info("Shutdown complete");

    return EXIT_SUCCESS;
}

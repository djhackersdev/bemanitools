#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs-ea3.h"
#include "imports/avs.h"

#include "launcher/avs-context.h"
#include "launcher/bootstrap-context.h"
#include "launcher/bs-config.h"
#include "launcher/ea3-config.h"
#include "launcher/module.h"
#include "launcher/options.h"
#include "launcher/property.h"
#include "launcher/stubs.h"
#include "launcher/version.h"

#include "util/defs.h"
#include "util/fs.h"
#include "util/log.h"
#include "util/os.h"
#include "util/str.h"

static void log_property_node_tree_rec(struct property_node *parent_node, const char* parent_path)
{
    char cur_path[4096];
    // 256 found in AVS code as size used on property_node_name
    char cur_node_name[256];
    char leaf_node_data[2048];
    struct property_node* child_node;

    // Carry on the full root path down the node tree
    property_node_name(parent_node, cur_node_name, sizeof(cur_node_name));

    str_cpy(cur_path, sizeof(cur_path), parent_path);
    str_cat(cur_path, sizeof(cur_path), "/");
    str_cat(cur_path, sizeof(cur_path), cur_node_name);

    child_node = property_node_traversal(parent_node, TRAVERSE_FIRST_CHILD);

    // parent node is a leaf node, print all data of it
    if (child_node == NULL) {
        property_node_read(parent_node, PROPERTY_TYPE_STR, leaf_node_data, sizeof(leaf_node_data));

        log_misc("%s: %s", cur_path, leaf_node_data);
    } else {
        while (child_node) {
            log_property_node_tree_rec(child_node, cur_path);

            child_node = property_node_traversal(child_node, TRAVERSE_NEXT_SIBLING);
        }
    }
}

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

static void log_property_tree(struct property *property)
{
    log_property_node_tree_rec(property_search(property, NULL, "/"), "");
}

static void log_property_node_tree(struct property_node *parent_node)
{
    log_property_node_tree_rec(parent_node, "");
}

static void log_launcher_and_env_info()
{
    char buffer_tmp[MAX_PATH];

    log_info(
        "launcher build date %s, gitrev %s",
        launcher_build_date,
        launcher_gitrev);
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

static void avs_initialize(struct bootstrap_config *bootstrap_config)
{
    struct property *avs_config;
    struct property_node *avs_config_root;

    log_misc("AVS initialize...");

    avs_config = boot_property_load(bootstrap_config->startup.avs_config_file);
    avs_config_root = property_search(avs_config, 0, "/config");

    log_property_tree(avs_config);

    if (avs_config_root == NULL) {
        log_fatal("%s: /config missing", bootstrap_config->startup.avs_config_file);
    }

    bootstrap_config_update_avs(bootstrap_config, avs_config_root);

    avs_context_init(
        avs_config,
        avs_config_root,
        bootstrap_config->startup.avs_heap_size,
        bootstrap_config->startup.std_heap_size,
        bootstrap_config->startup.log_file);

    boot_property_free(avs_config);

    log_misc("AVS logger available, switching to AVS loggers");

    log_to_external(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);
}

static void load_hook_dlls(struct array *hook_dlls)
{
    const char *hook_dll;

    for (size_t i = 0; i < hook_dlls->nitems; i++) {
        hook_dll = *array_item(char *, hook_dlls, i);

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

int main(int argc, const char **argv)
{
    bool ok;

    struct ea3_ident ea3;
    struct module_context module;
    struct options options;
    struct bootstrap_config bootstrap_config;

    struct property *app_config = NULL;
    struct property *ea3_config;

    struct property_node *app_config_root;
    struct property_node *ea3_config_root;

    // Static logging setup until we got AVS up and running
    log_to_writer(log_writer_file, stdout);
    log_set_level(LOG_LEVEL_MISC);

    log_launcher_and_env_info();

    log_misc("Reading command line options...");

    options_init(&options);

    if (!options_read_cmdline(&options, argc, argv) ||
           (!options.bootstrap_selector && !options.module)) {
        options_print_usage();

        return EXIT_FAILURE;
    }

    /* If enabled, wait for a remote debugger to attach. Spawning launcher
       with a debugger crashes it for some reason (e.g. on jubeat08). However,
       starting the launcher separately and attaching a remote debugger works */

    if (options.remote_debugger) {
        trap_remote_debugger();
    }

    log_misc("Loading before hook dlls...");

    load_hook_dlls(&options.before_hook_dlls);

    // Launcher supports two major boot modes: "manual" vs. bootstrap.xml
    if (!options.bootstrap_selector) {
        bootstrap_context_init(
            options.avs_config_path,
            options.ea3_config_path,
            options.std_heap_size,
            options.avs_heap_size,
            options.logfile,
            options.module,
            &bootstrap_config);
    } else {
        bootstrap_context_init_from_file(
            options.bootstrap_config_path,
            options.bootstrap_selector,
            &bootstrap_config);
    }

    avs_initialize(&bootstrap_config);
    
    bootstrap_context_post_avs_setup(&bootstrap_config);
  
    /* Load game DLL */

    if (options.iat_hook_dlls.nitems > 0) {
        module_context_init_with_iat_hooks(
            &module, options.module, &options.iat_hook_dlls);
    } else {
        module_context_init(&module, options.module);
    }

    /* Load hook DLLs */

    load_hook_dlls(&options.hook_dlls);

    /* Inject GetModuleHandle hooks */

    stubs_init();

    /* Prepare ea3 config */

    log_misc("Preparing ea3 configuration...");

    ea3_config = boot_property_load_avs(bootstrap_config.startup.eamuse_config_file);
    ea3_config_root = property_search(ea3_config, 0, "/ea3");

    if (ea3_config_root == NULL) {
        log_fatal("%s: /ea3 missing", bootstrap_config.startup.eamuse_config_file);
    }

    if (path_exists(options.ea3_ident_path)) {
        log_info("%s: loading override", options.ea3_ident_path);
        struct property *ea3_ident = boot_property_load(options.ea3_ident_path);
        struct property_node *node =
            property_search(ea3_ident, NULL, "/ea3_conf");
        if (node == NULL) {
            log_fatal("%s: /ea3_conf missing", options.ea3_ident_path);
        }

        for (node = property_node_traversal(node, TRAVERSE_FIRST_CHILD); node;
             node = property_node_traversal(node, TRAVERSE_NEXT_SIBLING)) {
            property_node_clone(NULL, ea3_config_root, node, TRUE);
        }

        boot_property_free(ea3_ident);
    }

    ea3_ident_init(&ea3);

    if (!ea3_ident_from_property(&ea3, ea3_config)) {
        log_fatal(
            "%s: Error reading IDs from config file", options.ea3_config_path);
    }

    if (options.softid != NULL) {
        str_cpy(ea3.softid, lengthof(ea3.softid), options.softid);
    }

    if (options.pcbid != NULL) {
        str_cpy(ea3.pcbid, lengthof(ea3.pcbid), options.pcbid);
    }

    if (!ea3.hardid[0]) {
        ea3_ident_hardid_from_ethernet(&ea3);
    }

    /* Invoke dll_entry_init */

    if (bootstrap_config.module_params) {
        app_config_root = bootstrap_config.module_params;
    } else if (path_exists(options.app_config_path)) {
        app_config = boot_property_load_avs(options.app_config_path);
        app_config_root = property_search(app_config, 0, "/param");
    } else {
        log_warning(
            "%s: app config file missing, using empty",
            options.app_config_path);
        app_config = boot_property_load_cstring("<param>dummy</param>");
        app_config_root = property_search(app_config, 0, "/param");
    }

    if (app_config_root == NULL) {
        log_fatal("%s: /param missing", options.app_config_path);
    }

    if (IsDebuggerPresent()) {
        /* Opportunity for breakpoint setup etc */
        DebugBreak();
    }

    ok = ea3_ident_invoke_module_init(&ea3, &module, app_config_root);

    if (!ok) {
        log_fatal("%s: dll_module_init() returned failure", options.module);
    }

    if (app_config) {
        boot_property_free(app_config);
    }

    ea3_ident_to_property(&ea3, ea3_config);

    if (options.override_urlslash_enabled) {
        log_info(
            "Overriding url_slash to: %d", options.override_urlslash_value);

        ea3_ident_replace_property_bool(
            ea3_config_root,
            "/network/url_slash",
            options.override_urlslash_value);
    }

    if (options.override_service != NULL) {
        log_info("Overriding service url to: %s", options.override_service);

        ea3_ident_replace_property_str(
            ea3_config_root, "/network/services", options.override_service);
    }

    /* Start up e-Amusement client */

    ea3_boot(ea3_config_root);
    boot_property_free(ea3_config);

    /* Run application */

    module_context_invoke_main(&module);

    /* Shut down */

    ea3_shutdown();

    log_to_writer(log_writer_file, stdout);
    avs_context_fini();

    module_context_fini(&module);
    options_fini(&options);

    return EXIT_SUCCESS;
}

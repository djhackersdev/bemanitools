#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs-ea3.h"
#include "imports/avs.h"

#include "launcher/avs-context.h"
#include "launcher/ea3-config.h"
#include "launcher/module.h"
#include "launcher/options.h"
#include "launcher/property.h"
#include "launcher/stubs.h"
#include "launcher/version.h"

#include "util/codepage.h"
#include "util/defs.h"
#include "util/fs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/os.h"
#include "util/str.h"

/* Gratuitous API changes orz */
static AVS_LOG_WRITER(log_callback, chars, nchars, ctx)
{
    wchar_t *utf16;
    char *utf8;
    int utf16_len;
    int utf8_len;
    int result;
    DWORD nwritten;
    HANDLE console;
    HANDLE file;

    /* Ignore existing NUL terminator */

    nchars--;

    /* Transcode shit_jis to UTF-8 */

    utf16_len = MultiByteToWideChar(CP_SHIFT_JIS, 0, chars, nchars, NULL, 0);

    if (utf16_len == 0) {
        abort();
    }

    utf16 = xmalloc(sizeof(*utf16) * utf16_len);
    result =
        MultiByteToWideChar(CP_SHIFT_JIS, 0, chars, nchars, utf16, utf16_len);

    if (result == 0) {
        abort();
    }

    utf8_len =
        WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_len, NULL, 0, NULL, NULL);

    if (utf8_len == 0) {
        abort();
    }

    utf8 = xmalloc(utf8_len + 2);
    result = WideCharToMultiByte(
        CP_UTF8, 0, utf16, utf16_len, utf8, utf8_len, NULL, NULL);

    if (result == 0) {
        abort();
    }

#if AVS_VERSION >= 1500
    utf8[utf8_len + 0] = '\r';
    utf8[utf8_len + 1] = '\n';

    utf8_len += 2;
#endif

    /* Write to console and log file */

    file = (HANDLE) ctx;
    console = GetStdHandle(STD_OUTPUT_HANDLE);

    if (ctx != INVALID_HANDLE_VALUE) {
        WriteFile(file, utf8, utf8_len, &nwritten, NULL);
    }

    WriteFile(console, utf8, utf8_len, &nwritten, NULL);

    /* Clean up */

    free(utf8);
    free(utf16);
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
    HANDLE logfile;

    struct ea3_ident ea3;
    struct module_context module;
    struct options options;

    struct property *app_config;
    struct property *avs_config;
    struct property *ea3_config;

    struct property_node *app_config_root;
    struct property_node *avs_config_root;
    struct property_node *ea3_config_root;

    log_to_writer(log_writer_file, stdout);
    log_info(
        "launcher build date %s, gitrev %s",
        launcher_build_date,
        launcher_gitrev);

    /* Read command line */

    options_init(&options);
    options_read_early_cmdline(&options, argc, argv);

    if (!options_read_cmdline(&options, argc, argv)) {
        options_print_usage();

        return EXIT_FAILURE;
    }

    /* If enabled, wait for a remote debugger to attach. Spawning launcher
       with a debugger crashes it for some reason (e.g. on jubeat08). However,
       starting the launcher separately and attaching a remote debugger works */

    if (options.remote_debugger) {
        log_info("Waiting until debugger attaches to remote process...");

        while (true) {
            BOOL res = FALSE;
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

    /* Start up AVS */

    if (options.logfile != NULL) {
        logfile = CreateFileA(
            options.logfile,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            0,
            NULL);
    } else {
        logfile = INVALID_HANDLE_VALUE;
    }

    avs_config = boot_property_load(options.avs_config_path);
    avs_config_root = property_search(avs_config, 0, "/config");

    if (avs_config_root == NULL) {
        log_fatal("%s: /config missing", options.avs_config_path);
    }

    load_hook_dlls(&options.before_hook_dlls);

    avs_context_init(
        avs_config_root,
        options.avs_heap_size,
        options.std_heap_size,
        log_callback,
        logfile);

    boot_property_free(avs_config);

    log_to_external(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    os_version_log();

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

    ea3_config = boot_property_load(options.ea3_config_path);
    ea3_config_root = property_search(ea3_config, 0, "/ea3");

    if (ea3_config_root == NULL) {
        log_fatal("%s: /ea3 missing", options.ea3_config_path);
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

    if (path_exists(options.app_config_path)) {
        app_config = boot_property_load(options.app_config_path);
    } else {
        log_warning(
            "%s: app config file missing, using empty",
            options.app_config_path);
        app_config = boot_property_load_cstring("<param>dummy</param>");
    }

    app_config_root = property_search(app_config, 0, "/param");

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

    boot_property_free(app_config);

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

    if (logfile != INVALID_HANDLE_VALUE) {
        CloseHandle(logfile);
    }

    module_context_fini(&module);
    options_fini(&options);

    return EXIT_SUCCESS;
}

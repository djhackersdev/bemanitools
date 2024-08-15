#define LOG_MODULE "inject"

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cconfig/cconfig-util.h"
#include "cconfig/cmd.h"

#include "core/boot.h"
#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-file.h"
#include "core/log-sink-list.h"
#include "core/log-sink-mutex.h"
#include "core/log-sink-std.h"
#include "core/thread-crt.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"

#include "inject/debugger.h"
#include "inject/options.h"
#include "inject/version.h"

#include "util/cmdline.h"
#include "util/debug.h"
#include "util/mem.h"
#include "util/os.h"
#include "util/signal.h"
#include "util/str.h"

static void _inject_log_header()
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
        "inject build date %s, gitrev %s", inject_build_date, inject_gitrev);
}

void _inject_log_init(
    const char *log_file_path, enum core_log_bt_log_level level)
{
    if (log_file_path) {
        core_log_bt_ext_init_async_with_stderr_and_file(log_file_path, false, true, 10);
    } else {
        core_log_bt_ext_init_async_with_stderr();
    }

    core_log_bt_core_api_set();

    core_log_bt_level_set(level);
}

static bool init_options(int argc, char **argv, struct options *options)
{
    options_init(options);

    if (argc < 3 || !options_read_cmdline(options, argc, argv)) {
        options_print_usage();
        return false;
    }

    return true;
}

static bool verify_hook_dll_and_exec_args_and_count_hooks(
    int argc, char **argv, uint32_t *hooks, uint32_t *exec_arg_pos)
{
    log_assert(argc >= 0);
    log_assert(argv);
    log_assert(hooks);
    log_assert(exec_arg_pos);

    *hooks = 0;
    *exec_arg_pos = 0;

    for (int i = 1; i < argc; i++) {
        if (str_ends_with(argv[i], "dll")) {
            (*hooks)++;
        } else if (str_ends_with(argv[i], "exe")) {
            *exec_arg_pos = i;
            break;
        }
    }

    if (!(*hooks)) {
        log_warning("ERROR: No Hook DLL(s) specified before executable");
        return false;
    }

    if (!*exec_arg_pos) {
        log_warning("ERROR: No executable specified");
        return false;
    }

    log_misc("%d hook(s) dll detected", *hooks);
    log_misc("Executable: %s", argv[*exec_arg_pos]);

    return true;
}

static bool
verify_hook_dlls_exist(int argc, char **argv, uint32_t hook_dll_count)
{
    log_assert(argc >= 0);
    log_assert(argv);

    char dll_path[MAX_PATH];
    DWORD dll_path_length;

    for (uint32_t i = 0; i < hook_dll_count; i++) {
        char *iat_hook = strstr(argv[i + 1], "=");

        if (iat_hook) {
            dll_path_length =
                SearchPath(NULL, iat_hook + 1, NULL, MAX_PATH, dll_path, NULL);
        } else {
            dll_path_length =
                SearchPath(NULL, argv[i + 1], NULL, MAX_PATH, dll_path, NULL);
        }

        if (dll_path_length == 0) {
            log_warning(
                "ERROR: Hook DLL not found: %08x",
                (unsigned int) GetLastError());

            return false;
        }
    }

    return true;
}

static bool inject_iat_hook_dlls(uint32_t hooks, char **argv)
{
    log_assert(argv);

    log_info("Injecting IAT hook DLLs...");

    for (int i = 0; i < hooks; i++) {
        char *iat_hook = strstr(argv[i + 1], "=");

        if (!iat_hook)
            continue;

        *iat_hook = '\0';
        debugger_replace_dll_iat(argv[i + 1], iat_hook + 1);
        *iat_hook = '=';
    }

    return true;
}

static bool inject_hook_dlls(uint32_t hooks, char **argv)
{
    log_assert(argv);

    log_info("Injecting hook DLLs...");

    for (int i = 0; i < hooks; i++) {
        char *iat_hook = strstr(argv[i + 1], "=");

        if (iat_hook)
            continue;

        if (!debugger_inject_dll(argv[i + 1])) {
            return false;
        }
    }

    return true;
}

static void signal_shutdown_handler()
{
    debugger_finit(true);
    core_log_bt_fini();
}

int main(int argc, char **argv)
{
    struct options options;
    uint32_t hooks;
    uint32_t exec_arg_pos;
    char *cmd_line;
    bool local_debugger;

    core_boot("inject");

    if (!init_options(argc, argv, &options)) {
        goto init_options_fail;
    }

    core_thread_crt_core_api_set();

    // TODO expose log level
    _inject_log_init(
        strlen(options.log_file) > 0 ? options.log_file : NULL,
        CORE_LOG_BT_LOG_LEVEL_MISC);

    _inject_log_header();
    os_version_log();

    signal_exception_handler_init();
    // Cleanup remote process on CTRL+C
    signal_register_shutdown_handler(signal_shutdown_handler);

    if (!verify_hook_dll_and_exec_args_and_count_hooks(
            argc, argv, &hooks, &exec_arg_pos)) {
        goto verify_fail;
    }

    if (!verify_hook_dlls_exist(argc, argv, hooks)) {
        goto verify_2_fail;
    }

    // buffer consumed by debugger_init
    cmd_line = args_join(argc - exec_arg_pos, argv + exec_arg_pos);

    local_debugger = options.debug && !options.remote_debugger;

    if (!debugger_init(local_debugger, argv[exec_arg_pos], cmd_line)) {
        goto debugger_init_fail;
    }

    if (!inject_iat_hook_dlls(hooks, argv)) {
        goto inject_hook_dlls_fail;
    }

    if (!inject_hook_dlls(hooks, argv)) {
        goto inject_hook_dlls_fail;
    }

    // Execute this after injecting the DLLs. Some debuggers seem to crash if we
    // attach the process before DLL injection (inject's local one doesn't
    // crash). However, this means the remote debugger is missing out on all
    // injected DLL loads, e.g. calls to DllMain
    if (options.remote_debugger) {
        if (!debugger_wait_for_remote_debugger()) {
            goto debugger_wait_for_remote_debugger_fail;
        }
    }

    if (!debugger_resume_process()) {
        goto debugger_resume_process_fail;
    }

    debugger_wait_process_exit();

    debugger_finit(false);

    core_log_bt_fini();

    return EXIT_SUCCESS;

debugger_resume_process_fail:
debugger_wait_for_remote_debugger_fail:
inject_hook_dlls_fail:
    debugger_finit(true);

debugger_init_fail:
verify_2_fail:
verify_fail:
    core_log_bt_fini();

init_options_fail:
    return EXIT_FAILURE;
}

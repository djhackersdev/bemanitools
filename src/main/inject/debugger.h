#pragma once

#include <stdbool.h>

typedef enum debugger_attach_type {
    DEBUGGER_ATTACH_TYPE_NONE = 0,
    DEBUGGER_ATTACH_TYPE_INJECT = 1,
    DEBUGGER_ATTACH_TYPE_EXTERNAL = 2,
} debugger_attach_type_t;

/**
 * Initialize the debugger.
 *
 * This creates the remote process of the provided application.
 *
 * The remote process is created suspended. This allows you to
 * to some more process setup tasks like injecting hook DLLs
 * before you call debugger_resume_process to actually start
 * execution of it.
 *
 * The actual debugging runs in a dedicated thread which spawns
 * the process, waits for and dispatches debug events.
 *
 * However, if you want to attach a remote debugger, you have to
 * set the parameter local_debugger to false. Then, the debugger
 * will only create the remote process and monitor it.
 *
 * @param attach_type Setup the debugger to attach with the given type of debugger
 * @param app_name Name of the application to spawn and debug.
 * @param cmd_line Command line string to pass to application.
 */
void debugger_init(debugger_attach_type_t attach_type, const char *app_name, const char *cmd_line);

/**
 * Inject a DLL into the remote process.
 *
 * @param path_dll Path to the dll to inject.
 * @return true if sucessful, false on error.
 */
bool debugger_inject_dll(const char *path_dll);

void debugger_run();

/**
 * Inject a DLL into the remote process by replacing its reference in
 * the import table.
 *
 * @param expected_dll Name of dll to override.
 * @param replacement_path_dll Name of dll to inject.
 * @return true if sucessful, false on error.
 */
bool debugger_replace_dll_iat(
    const char *expected_dll, const char *replacement_path_dll);

/**
 * Wait/block for a remote debugger to attach to the remote process.
 *
 * You only need to call this if you specified local_debugger = false
 * on debugger_init.
 *
 * @return True if successfull and a remote debugger attached, false
 *         on error.
 */
bool debugger_wait_for_remote_debugger();

/**
 * Resume the remote process.
 *
 * Make sure to call this once you are done with setting it up.
 *
 * @return true on success, false on error.
 */
bool debugger_resume_process();

/**
 * Wait for the remote process to exit.
 */
void debugger_wait_process_exit();

/**
 * Cleanup the debugger.
 *
 * @param failure Set this to true if you have to cleanup due to
 *                a failure of another debugger function call.
 *                Otherwise, set this to false.
 */
void debugger_finit(bool failure);
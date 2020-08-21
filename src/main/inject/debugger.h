#pragma once

#include <stdbool.h>


/**
 * Initialize inject's logger backend.
 * 
 * This takes care of hooking and merging the different log
 * streams, e.g. inject's local logging and inject's debugger
 * receiving remote logging events.
 * 
 * @param log_file_path Path to the file to log to or NULL to
 *                      disable.
 */


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
 * @param local_debugger True to attach inject's local debugger,
 *                       false to allow attaching a remote
 *                       debugger with enhanced features.
 * @param app_name Name of the application to spawn and debug.
 * @param cmd_line Command line string to pass to application.
 * @return true on success, false on error. On error, no remote
 *         application and local debugger is started.
 */
bool debugger_init(bool local_debugger, const char *app_name, char *cmd_line);

/**
 * Inject a DLL into the remote process.
 * 
 * @param path_dll Path to the dll to inject.
 * @return true if sucessful, false on error.
 */
bool debugger_inject_dll(const char *path_dll);

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
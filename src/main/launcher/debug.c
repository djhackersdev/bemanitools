
#define LOG_MODULE "debug"

#include <stdbool.h>
#include <windows.h>

#include "core/log.h"

#include "launcher/debug.h"

void debug_remote_debugger_trap()
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
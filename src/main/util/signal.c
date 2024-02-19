#include <unistd.h>

#include "util/hex.h"
#include "util/log.h"
#include "util/signal.h"

static signal_shutdown_handler_t shutdown_handler;

static const char *control_code_to_str(DWORD ctrl_code)
{
    switch (ctrl_code) {
        case CTRL_C_EVENT:
            return "CTRL_C_EVENT";
        case CTRL_BREAK_EVENT:
            return "CTRL_BREAK_EVENT";
        case CTRL_CLOSE_EVENT:
            return "CTRL_CLOSE_EVENT";
        case CTRL_LOGOFF_EVENT:
            return "CTRL_LOGOFF_EVENT";
        case CTRL_SHUTDOWN_EVENT:
            return "CTRL_SHUTDOWN_EVENT";
        default:
            log_warning("Unknown control code: %lX", ctrl_code);
            return "Unknown";
    }
}

static BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType)
{
    log_misc(
        "Console ctrl handler called: %s", control_code_to_str(dwCtrlType));

    if (dwCtrlType == CTRL_C_EVENT) {
        if (shutdown_handler) {
            log_misc("Executing shutdown handler");
            shutdown_handler();
        }

        log_info("Exiting process");

        ExitProcess(0);
    }

    return FALSE;
}

void signal_exception_handler_init()
{
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

    log_info("Initialized");
}

void signal_register_shutdown_handler(signal_shutdown_handler_t handler)
{
    shutdown_handler = handler;

    log_misc("Registered shutdown handler");
}
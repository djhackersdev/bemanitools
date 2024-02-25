#include <unistd.h>
#include <windows.h>

#include "core/log.h"

#include "util/hex.h"
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

static LONG WINAPI
unhandled_exception_filter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    // no exception info provided
    if (ExceptionInfo != NULL) {
        struct _EXCEPTION_RECORD *ExceptionRecord =
            ExceptionInfo->ExceptionRecord;

        log_warning(
            "Exception raised: %s",
            signal_exception_code_to_str(ExceptionRecord->ExceptionCode));

        struct _EXCEPTION_RECORD *record_cause =
            ExceptionRecord->ExceptionRecord;

        while (record_cause != NULL) {
            log_warning(
                "Caused by: %s",
                signal_exception_code_to_str(record_cause->ExceptionCode));
            record_cause = record_cause->ExceptionRecord;
        }

        // TODO print stacktrace

        log_fatal("End exception handler");
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

void signal_exception_handler_init()
{
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
    SetUnhandledExceptionFilter(unhandled_exception_filter);

    log_info("Initialized");
}

void signal_register_shutdown_handler(signal_shutdown_handler_t handler)
{
    shutdown_handler = handler;

    log_misc("Registered shutdown handler");
}

const char *signal_exception_code_to_str(DWORD code)
{
    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:
            return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:
            return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT:
            return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION:
            return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW:
            return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK:
            return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW:
            return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR:
            return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:
            return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION:
            return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION:
            return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP:
            return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW:
            return "EXCEPTION_STACK_OVERFLOW";
        case DBG_CONTROL_C:
            return "DBG_CONTROL_C";
        default:
            log_warning("Unknown exception code: %lX", code);
            return "Unknown";
    }
}
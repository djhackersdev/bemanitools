#define LOG_MODULE "exceptiontrace"

#include <windows.h>

#include <stdio.h>

#include "core/log.h"

#include "imports/dwarfstack.h"

#include "util/debug.h"

#define log_exception(...) _debug_exception_msg("exception", __VA_ARGS__)

static core_log_message_t _debug_exception_msg;

static void _debug_stacktrace_printer(
    uint64_t addr,
    const char *filename,
    int lineno,
    const char *funcname,
    void *context,
    int columnno)
{
    int *count;
    const char *delim;
    void *ptr;
    char buffer[512];
    char *buffer_ptr;

    count = context;
    delim = strrchr(filename, '/');

    if (delim) {
        filename = delim + 1;
    }

    delim = strrchr(filename, '\\');

    if (delim) {
        filename = delim + 1;
    }

    ptr = (void *) (uintptr_t) addr;

    switch (lineno) {
        case DWST_BASE_ADDR:
            log_exception("base address: 0x%p (%s)", ptr, filename);
            break;

        case DWST_NOT_FOUND:
        case DWST_NO_DBG_SYM:
        case DWST_NO_SRC_FILE:
            log_exception(
                "    stack %02d: 0x%p (%s)", (*count)++, ptr, filename);
            break;

        default:
            buffer_ptr = buffer;
            memset(buffer, 0, sizeof(buffer));

            if (ptr) {
                buffer_ptr += sprintf(
                    buffer_ptr, "    stack %02d: 0x%p", (*count)++, ptr);
            } else {
                buffer_ptr += sprintf(
                    buffer_ptr,
                    "                %*s",
                    (int) sizeof(void *) * 2,
                    "");
            }

            buffer_ptr += sprintf(buffer_ptr, " (%s:%d", filename, lineno);

            if (columnno > 0) {
                buffer_ptr += sprintf(buffer_ptr, ":%d", columnno);
            }

            buffer_ptr += sprintf(buffer_ptr, ")");

            if (funcname) {
                buffer_ptr += sprintf(buffer_ptr, " [%s]", funcname);
            }

            log_exception(buffer);

            break;
    }
}

static LONG WINAPI _debug_unhandled_exception_filter(LPEXCEPTION_POINTERS ep)
{
    DWORD code;
    const char *desc;
    ULONG_PTR flag;
    ULONG_PTR addr;
    int count;

    log_exception("==========================================================");
    log_exception("The application has crashed due to an unhandled exception!");

    code = ep->ExceptionRecord->ExceptionCode;
    desc = debug_exception_code_to_str(code);

    log_exception("code: 0x%08lX", code);
    log_exception("desc: %s", desc);

    if (code == EXCEPTION_ACCESS_VIOLATION &&
        ep->ExceptionRecord->NumberParameters == 2) {
        flag = ep->ExceptionRecord->ExceptionInformation[0];
        addr = ep->ExceptionRecord->ExceptionInformation[1];

        log_exception(
            "%s violation at 0x%p",
            flag == 8 ? "data execution prevention" :
                        (flag ? "write access" : "read access"),
            (void *) addr);
    }

    log_exception("stacktrace:");

    count = 0;

    dwstOfException(ep->ContextRecord, &_debug_stacktrace_printer, &count);

    log_exception("End of stacktrace");
    log_exception("==========================================================");

    return EXCEPTION_EXECUTE_HANDLER;
}

void debug_init(core_log_message_t exception_msg)
{
    _debug_exception_msg = exception_msg;
    SetUnhandledExceptionFilter(_debug_unhandled_exception_filter);

    log_info("Initialized");
}

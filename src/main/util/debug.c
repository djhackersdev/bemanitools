#define LOG_MODULE "util-debug"

#include <windows.h>

#include <stdio.h>

#include "imports/dwarfstack.h"

#include "util/debug.h"
#include "util/log.h"

#define EX_DESC( name ) \
    case EXCEPTION_##name: desc = " (" #name ")"; \
                           break

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
    char* buffer_ptr;
    
    count = context;
    delim = strrchr(filename, '/');

    if (delim) {
        filename = delim + 1;
    }
    
    delim = strrchr(filename, '\\');
    
    if (delim) {
        filename = delim + 1;
    }

    ptr = (void*)(uintptr_t) addr;

    switch (lineno) {
        case DWST_BASE_ADDR:
            log_exception_handler("base address: 0x%p (%s)", ptr, filename);
            break;

        case DWST_NOT_FOUND:
        case DWST_NO_DBG_SYM:
        case DWST_NO_SRC_FILE:
            log_exception_handler("    stack %02d: 0x%p (%s)", (*count)++, ptr, filename);
            break;

        default:
            buffer_ptr = buffer;
            memset(buffer, 0, sizeof(buffer));

            if (ptr) {
                buffer_ptr += sprintf(buffer_ptr, "    stack %02d: 0x%p", (*count)++, ptr);
            } else {
                buffer_ptr += sprintf(buffer_ptr, "                %*s", (int) sizeof(void*) * 2, "");
            }

            buffer_ptr += sprintf(buffer_ptr, " (%s:%d", filename, lineno);
            
            if (columnno > 0) {
                buffer_ptr += sprintf(buffer_ptr, ":%d", columnno);
            }

            buffer_ptr += sprintf(buffer_ptr, ")");
            
            if (funcname) {
                buffer_ptr += sprintf(buffer_ptr, " [%s]", funcname);
            }

            log_exception_handler(buffer);

            break;
    }
}

static LONG WINAPI
_debug_unhandled_exception_filter(LPEXCEPTION_POINTERS ep)
{
    DWORD code;
    const char *desc;
    ULONG_PTR flag;
    ULONG_PTR addr;
    int count;

    log_exception_handler("==========================================================");
    log_exception_handler("The application has crashed due to an unhandled exception!");

    code = ep->ExceptionRecord->ExceptionCode;
    desc = debug_exception_code_to_str(code);

    log_exception_handler("code: 0x%08lX", code);
    log_exception_handler("desc: %s", desc);

    if (code == EXCEPTION_ACCESS_VIOLATION &&
            ep->ExceptionRecord->NumberParameters == 2) {
        flag = ep->ExceptionRecord->ExceptionInformation[0];
        addr = ep->ExceptionRecord->ExceptionInformation[1];

        log_exception_handler("%s violation at 0x%p",
            flag == 8 ? "data execution prevention" :
            (flag ? "write access":"read access"), (void*) addr);
    }

    log_exception_handler("stacktrace:");

    count = 0;

    dwstOfException(ep->ContextRecord, &_debug_stacktrace_printer, &count);

    return EXCEPTION_EXECUTE_HANDLER;
}

void debug_init()
{
    SetUnhandledExceptionFilter(_debug_unhandled_exception_filter);

    log_info("Initialized");
}

void debug_print_stacktrace()
{
    int count;

    count = 0;

    log_exception_handler("==========================================================");
    log_exception_handler("Debug stacktrace");

    dwstOfLocation(&_debug_stacktrace_printer, &count);
}

const char *debug_exception_code_to_str(DWORD code)
{
    const char *desc = "";

    switch (code) {
        EX_DESC(ACCESS_VIOLATION);
        EX_DESC(ARRAY_BOUNDS_EXCEEDED);
        EX_DESC(BREAKPOINT);
        EX_DESC(DATATYPE_MISALIGNMENT);
        EX_DESC(FLT_DENORMAL_OPERAND);
        EX_DESC(FLT_DIVIDE_BY_ZERO);
        EX_DESC(FLT_INEXACT_RESULT);
        EX_DESC(FLT_INVALID_OPERATION);
        EX_DESC(FLT_OVERFLOW);
        EX_DESC(FLT_STACK_CHECK);
        EX_DESC(FLT_UNDERFLOW);
        EX_DESC(ILLEGAL_INSTRUCTION);
        EX_DESC(IN_PAGE_ERROR);
        EX_DESC(INT_DIVIDE_BY_ZERO);
        EX_DESC(INT_OVERFLOW);
        EX_DESC(INVALID_DISPOSITION);
        EX_DESC(NONCONTINUABLE_EXCEPTION);
        EX_DESC(PRIV_INSTRUCTION);
        EX_DESC(SINGLE_STEP);
        EX_DESC(STACK_OVERFLOW);
        case DBG_CONTROL_C:
            return "DBG_CONTROL_C";
        default:
            log_warning("Unknown exception code: %lX", code);
            return "Unknown";
    }

    return desc;
}
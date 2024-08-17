#define LOG_MODULE "util-debug"

#include <windows.h>

#include <stdio.h>

#include "iface-core/log.h"

#define EX_DESC(name)          \
    case EXCEPTION_##name:     \
        desc = " (" #name ")"; \
        break

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
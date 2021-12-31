#define LOG_MODULE "jbhook-locale"

#include <windows.h>

#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"

// ANSI/OEM Japanese; Japanese (Shift-JIS)
#define CODEPAGE_SHIFT_JIS 932

static UINT WINAPI hook_GetACP();
static UINT WINAPI hook_GetOEMCP();

static const struct hook_symbol locale_hook_syms[] = {
    {.name = "GetOEMCP",
     .patch = hook_GetACP},
    {.name = "GetOEMCP",
     .patch = hook_GetOEMCP},
};

static UINT WINAPI hook_GetACP() {
    return CODEPAGE_SHIFT_JIS;
}

static UINT WINAPI hook_GetOEMCP() {
    return CODEPAGE_SHIFT_JIS;
}


void jbhook_util_locale_hook_init(void)
{
    hook_table_apply(NULL, "kernel32.dll", locale_hook_syms, lengthof(locale_hook_syms));

    log_info("Inserted locale hooks");
}

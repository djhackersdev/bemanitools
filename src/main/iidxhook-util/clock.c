#define LOG_MODULE "clock-hook"

#include <windows.h>

#include "hook/table.h"

#include "iface-core/log.h"

#include "util/defs.h"

static BOOL STDCALL my_SetLocalTime(const SYSTEMTIME *lpSystemTime);
static BOOL(STDCALL *real_SetLocalTime)(const SYSTEMTIME *lpSystemTime);

static const struct hook_symbol clock_hook_syms[] = {
    {.name = "SetLocalTime",
     .patch = my_SetLocalTime,
     .link = (void **) &real_SetLocalTime},
};

static BOOL STDCALL my_SetLocalTime(const SYSTEMTIME *lpSystemTime)
{
    /* Stub, don't mess with system clock */
    log_misc("Blocked setting system clock time");
    return TRUE;
}

void iidxhook_util_clock_hook_init(void)
{
    hook_table_apply(
        NULL, "kernel32.dll", clock_hook_syms, lengthof(clock_hook_syms));

    log_info("Inserted clock hooks");
}

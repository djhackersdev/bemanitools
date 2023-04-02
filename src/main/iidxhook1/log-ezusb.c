#define LOG_MODULE "log-ezusb"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/table.h"

#include "iidxhook1/log-ezusb.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

/* ------------------------------------------------------------------------- */

static int STDCALL my_wvsprintfA(LPSTR lpOut, LPCSTR lpFmt, va_list args);

static int(STDCALL *real_wvsprintfA)(LPSTR lpOut, LPCSTR lpFmt, va_list args);

/* ------------------------------------------------------------------------- */

static const struct hook_symbol ezusb_log_hook_syms[] = {
    {.name = "wvsprintfA",
     .patch = my_wvsprintfA,
     .link = (void **) &real_wvsprintfA},
};

/* ------------------------------------------------------------------------- */

static int STDCALL my_wvsprintfA(LPSTR lpOut, LPCSTR lpFmt, va_list args)
{
    char buf[5192];
    vsprintf(buf, lpFmt, args);
    log_info("%s", buf);

    return real_wvsprintfA(lpOut, lpFmt, args);
}

/* ------------------------------------------------------------------------- */

void ezusb_log_hook_init(void)
{
    hook_table_apply(
        NULL, "user32.dll", ezusb_log_hook_syms, lengthof(ezusb_log_hook_syms));

    log_info("Inserted ezusb log hooks");
}

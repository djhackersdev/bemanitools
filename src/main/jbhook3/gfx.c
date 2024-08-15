#define LOG_MODULE "jbhook-gfx"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hook/table.h"

#include "iface-core/log.h"

#include "jbhook3/options.h"

static bool jbhook3_gfx_windowed;
static bool jbhook3_gfx_show_cursor;

void jbhook3_gfx_set_windowed(void)
{
    jbhook3_gfx_windowed = true;
}

void jbhook3_gfx_set_show_cursor(void)
{
    jbhook3_gfx_show_cursor = true;
}

static HWND CDECL
my_GFWin32MainWindowRun(HINSTANCE, const char *, long, long, DWORD);

static HWND(CDECL *real_GFWin32MainWindowRun)(
    HINSTANCE, const char *, long, long, DWORD);

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "GFWin32MainWindowRun",
        .patch = my_GFWin32MainWindowRun,
        .link = (void **) &real_GFWin32MainWindowRun,
    },
};

static HWND CDECL my_GFWin32MainWindowRun(
    HINSTANCE hInstance,
    const char *lpWindowName,
    long X,
    long Y,
    DWORD dwStyle)
{
    if (jbhook3_gfx_windowed) {
        log_info("--- Begin jbhook GFWin32MainWindowRun ---");
        dwStyle |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
    }

    if (jbhook3_gfx_show_cursor) {
        ShowCursor(TRUE);
    }

    return real_GFWin32MainWindowRun(hInstance, lpWindowName, X, Y, dwStyle);
}

void jbhook3_gfx_init(void)
{
    hook_table_apply(
        NULL, "gftools.dll", init_hook_syms, lengthof(init_hook_syms));

    log_info("Inserted gfx hooks");
}
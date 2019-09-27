#define LOG_MODULE "jbhook-gfx"

#include <windows.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "jbhook/gfx.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

static DWORD STDCALL my_GetGlyphOutline(HDC hdc, UINT uChar, UINT uFormat,
        LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer,
        const MAT2* lpmat2);

static DWORD (STDCALL* real_GetGlyphOutline)(HDC hdc, UINT uChar, UINT uFormat,
        LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer,
        const MAT2* lpmat2);

static const struct hook_symbol gfx_hook_syms[] = {
    {
        .name       = "GetGlyphOutlineA",
        .patch      = my_GetGlyphOutline,
        .link       = (void **) &real_GetGlyphOutline
    },
    {
        .name       = "IsDBCSLeadByteEx",
        .patch      = my_GetGlyphOutline,               // !??
        .link       = (void **) &real_GetGlyphOutline   // !??
    },
};

static DWORD STDCALL my_GetGlyphOutline(HDC hdc, UINT uChar, UINT uFormat,
        LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer,
        const MAT2* lpmat2)
{
    if (IsDBCSLeadByteEx(CP_ACP, uChar & 0xFF)) {
        log_warning("!!!!!!!!");

        char tmp[2];

        tmp[0] = uChar & 0xFF;
        tmp[1] = (uChar >> 8) & 0xFF;

        MultiByteToWideChar(CP_ACP, MB_USEGLYPHCHARS, tmp, 2, lpvBuffer, cbBuffer);
    }

    return real_GetGlyphOutline(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer,
        lpmat2);
}

void gfx_hook_init(void)
{
    hook_table_apply(
            NULL,
            "gdi32.dll",
            gfx_hook_syms,
            lengthof(gfx_hook_syms));

    log_info("Inserted gfx hooks");
}

void gfx_set_windowed(bool framed)
{
}


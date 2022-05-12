// TODO: Fix gray arrows on NVIDIA cards (also applies to X3)

#define LOG_MODULE "d3d9-hook"

#include <d3d9.h>
#include <d3dx9core.h>
#include <windows.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "ddrhookx/d3d9.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

static LONG(STDCALL *real_ChangeDisplaySettingsExA)(
    char *dev_name, DEVMODE *dev_mode, HWND hwnd, DWORD flags, void *param);
static LONG(STDCALL *real_SetWindowLongW)(
    HWND hWnd, int nIndex, LONG dwNewLong);
static BOOL(STDCALL *real_SetWindowPos)(
    HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

static LONG STDCALL my_ChangeDisplaySettingsExA(
    char *dev_name, DEVMODE *dev_mode, HWND hwnd, DWORD flags, void *param);
static LONG STDCALL my_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong);
static BOOL STDCALL my_SetWindowPos(
    HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

static struct ddrhookx_d3d9_config ddrhookx_d3d9_config;

static const struct hook_symbol misc_user32_syms[] = {
    {
        .name = "ChangeDisplaySettingsExA",
        .patch = my_ChangeDisplaySettingsExA,
        .link = (void **) &real_ChangeDisplaySettingsExA,
    },
    {
        .name = "SetWindowLongW",
        .patch = my_SetWindowLongW,
        .link = (void **) &real_SetWindowLongW,
    },
    {
        .name = "SetWindowPos",
        .patch = my_SetWindowPos,
        .link = (void **) &real_SetWindowPos,
    },
};

/* ------------------------------------------------------------------------- */

static void ddrhookx_d3d9_calc_win_size_with_framed(
    HWND hwnd, DWORD x, DWORD y, DWORD width, DWORD height, LPWINDOWPOS wp)
{
    /* taken from dxwnd */
    RECT rect;
    DWORD style;
    int max_x, max_y;
    HMENU menu;

    rect.left = x;
    rect.top = y;
    max_x = width;
    max_y = height;
    rect.right = x + max_x;
    rect.bottom = y + max_y;

    style = GetWindowLong(hwnd, GWL_STYLE);
    menu = GetMenu(hwnd);
    AdjustWindowRect(&rect, style, (menu != NULL));

    /* shift down-right so that the border is visible
       and also update the iPosX,iPosY upper-left coordinates
       of the client area */

    if (rect.left < 0) {
        rect.right -= rect.left;
        rect.left = 0;
    }

    if (rect.top < 0) {
        rect.bottom -= rect.top;
        rect.top = 0;
    }

    wp->x = rect.left;
    wp->y = rect.top;
    wp->cx = rect.right - rect.left;
    wp->cy = rect.bottom - rect.top;
}

/* ------------------------------------------------------------------------- */

static LONG STDCALL my_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
    if (ddrhookx_d3d9_config.windowed && nIndex == GWL_STYLE) {
        dwNewLong |= WS_OVERLAPPEDWINDOW;
    }

    return real_SetWindowLongW(hWnd, nIndex, dwNewLong);
}

static BOOL STDCALL my_SetWindowPos(
    HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    if (ddrhookx_d3d9_config.windowed) {
        WINDOWPOS wp;

        ddrhookx_d3d9_calc_win_size_with_framed(hWnd, X, Y, cx, cy, &wp);

        return real_SetWindowPos(
            hWnd, hWndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, uFlags);
    }

    return real_SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

static LONG STDCALL my_ChangeDisplaySettingsExA(
    char *dev_name, DEVMODE *dev_mode, HWND hwnd, DWORD flags, void *param)
{
    if (ddrhookx_d3d9_config.windowed) {
        return DISP_CHANGE_SUCCESSFUL;
    }

    return real_ChangeDisplaySettingsExA(
        dev_name, dev_mode, hwnd, flags, param);
}

/* ------------------------------------------------------------------------- */

static void
ddrhookx_d3d9_log_config(const struct ddrhookx_d3d9_config *config)
{
    log_misc(
        "ddrhookx_d3d9_config\n"
        "windowed: %d\n",
        config->windowed);
}

void ddrhookx_d3d9_init_config(struct ddrhookx_d3d9_config *config)
{
    config->windowed = false;
}

void ddrhookx_d3d9_hook_init()
{
    ddrhookx_d3d9_init_config(&ddrhookx_d3d9_config);

    hook_table_apply(
        NULL, "user32.dll", misc_user32_syms, lengthof(misc_user32_syms));

    log_info("Initialized d3d9 hooks");
}

/* ------------------------------------------------------------------------- */

void ddrhookx_d3d9_configure(
    const struct ddrhookx_d3d9_config *config)
{
    log_assert(config);

    ddrhookx_d3d9_log_config(config);

    memcpy(
        &ddrhookx_d3d9_config,
        config,
        sizeof(struct ddrhookx_d3d9_config));
}

static void ddrhookx_d3d9_patch_window(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX);

    if (ddrhookx_d3d9_config.windowed) {
        /* use a different style */
        irp->args.create_window_ex.style |= WS_OVERLAPPEDWINDOW;

        /* also show mouse cursor */
        ShowCursor(TRUE);
    }
}

static void
ddrhookx_d3d9_fix_window_size_and_pos(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX);

    if (ddrhookx_d3d9_config.windowed) {
        /* we have to adjust the window size, because the window needs to be a
           slightly bigger than the rendering resolution (window caption and
           stuff is included in the window size) */
        WINDOWPOS wp;

        ddrhookx_d3d9_calc_win_size_with_framed(
            irp->args.create_window_ex.result,
            irp->args.create_window_ex.x,
            irp->args.create_window_ex.y,
            irp->args.create_window_ex.width,
            irp->args.create_window_ex.height,
            &wp);

        SetWindowPos(
            irp->args.create_window_ex.result, 0, wp.x, wp.y, wp.cx, wp.cy, 0);

        irp->args.create_window_ex.x = wp.x;
        irp->args.create_window_ex.y = wp.y;
        irp->args.create_window_ex.width = wp.cx;
        irp->args.create_window_ex.height = wp.cy;
    }
}

static void ddrhookx_d3d9_create_device_apply_window_mode(
    struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);
    D3DPRESENT_PARAMETERS *pp = irp->args.ctx_create_device.pp;

    if (ddrhookx_d3d9_config.windowed) {
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    }
}

static void ddrhookx_d3d9_reset_apply_window_mode(
    struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_RESET);
    D3DPRESENT_PARAMETERS *pp = irp->args.dev_reset.pp;

    if (ddrhookx_d3d9_config.windowed) {
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    }
}

HRESULT
ddrhookx_d3d9_irp_handler(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);

    switch (irp->op) {
        case HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX:
            ddrhookx_d3d9_patch_window(irp);

            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                ddrhookx_d3d9_fix_window_size_and_pos(irp);
            }

            return hr;

        case HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE:
            ddrhookx_d3d9_create_device_apply_window_mode(irp);
            hr = hook_d3d9_irp_invoke_next(irp);
            return hr;

        case HOOK_D3D9_IRP_OP_DEV_RESET:
            ddrhookx_d3d9_reset_apply_window_mode(irp);
            hr = hook_d3d9_irp_invoke_next(irp);
            return hr;

        default:
            return hook_d3d9_irp_invoke_next(irp);
    }

    log_fatal("Illegal state");
}
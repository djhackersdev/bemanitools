#define LOG_MODULE "d3d9ex-hook"

#include <d3d9.h>
#include <d3dx9core.h>
#include <windows.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "d3d9exhook/d3d9ex.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

/* ------------------------------------------------------------------------- */

static HWND STDCALL my_CreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

static HRESULT STDCALL my_CreateDeviceEx(
    IDirect3D9Ex *self,
    UINT adapter,
    D3DDEVTYPE type,
    HWND hwnd,
    DWORD flags,
    D3DPRESENT_PARAMETERS *pp,
    D3DDISPLAYMODEEX *fdm,
    IDirect3DDevice9Ex **pdev);

static HRESULT STDCALL my_Direct3DCreate9Ex(UINT sdk_ver, IDirect3D9Ex **api);

static BOOL STDCALL my_EnumDisplayDevicesA(
    const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info, DWORD flags);

static BOOL STDCALL
my_MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);

static void calc_win_size_with_framed(
    HWND hwnd, DWORD x, DWORD y, DWORD width, DWORD height, LPWINDOWPOS wp);

static HWND(STDCALL *real_CreateWindowExA)(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

static HRESULT(STDCALL *real_Direct3DCreate9Ex)(
    UINT sdk_ver, IDirect3D9Ex **api);

static BOOL(STDCALL *real_EnumDisplayDevicesA)(
    const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info, DWORD flags);

static BOOL(STDCALL *real_MoveWindow)(
    HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);

/* ------------------------------------------------------------------------- */

static bool d3d9ex_windowed;
static int32_t d3d9ex_force_refresh_rate = -1;
static int32_t d3d9ex_window_width = -1;
static int32_t d3d9ex_window_height = -1;
static bool d3d9ex_window_framed;
static int32_t d3d9ex_device_adapter = -1;

/* ------------------------------------------------------------------------- */

static const struct hook_symbol d3d9ex_hook_syms[] = {
    {.name = "Direct3DCreate9Ex",
     .patch = my_Direct3DCreate9Ex,
     .link = (void **) &real_Direct3DCreate9Ex},
};

static const struct hook_symbol d3d9ex_hook_user32_syms[] = {
    {.name = "EnumDisplayDevicesA",
     .patch = my_EnumDisplayDevicesA,
     .link = (void **) &real_EnumDisplayDevicesA},
    {.name = "CreateWindowExA",
     .patch = my_CreateWindowExA,
     .link = (void **) &real_CreateWindowExA},
    {.name = "MoveWindow",
     .patch = my_MoveWindow,
     .link = (void **) &real_MoveWindow},
};

/* ------------------------------------------------------------------------- */

static HWND STDCALL my_CreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    if (d3d9ex_windowed && d3d9ex_window_framed) {
        /* use a different style */
        dwStyle |= WS_OVERLAPPEDWINDOW;
        /* also show mouse cursor */
        ShowCursor(TRUE);
    }

    HWND hwnd = real_CreateWindowExA(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,
        X,
        Y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam);

    return hwnd;
}

static BOOL STDCALL
my_MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
    if (d3d9ex_windowed && d3d9ex_window_framed) {
        /* we have to adjust the window size, because the window needs to be a
           slightly bigger than the rendering resolution (window caption and
           stuff is included in the window size) */

        if (d3d9ex_window_width != -1 && d3d9ex_window_height != -1) {
            log_misc(
                "Overriding window size from %dx%d with %dx%d",
                nWidth,
                nHeight,
                d3d9ex_window_width,
                d3d9ex_window_height);

            nWidth = d3d9ex_window_width;
            nHeight = d3d9ex_window_height;
        }

        WINDOWPOS wp;
        calc_win_size_with_framed(hWnd, X, Y, nWidth, nHeight, &wp);
        SetWindowPos(hWnd, 0, wp.x, wp.y, wp.cx, wp.cy, 0);
        X = wp.x;
        Y = wp.y;
        nWidth = wp.cx;
        nHeight = wp.cy;
    }

    BOOL result = real_MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

    return result;
}

static HRESULT STDCALL my_CreateDeviceEx(
    IDirect3D9Ex *self,
    UINT adapter,
    D3DDEVTYPE type,
    HWND hwnd,
    DWORD flags,
    D3DPRESENT_PARAMETERS *pp,
    D3DDISPLAYMODEEX *fdm,
    IDirect3DDevice9Ex **pdev)
{
    IDirect3D9Ex *real = COM_PROXY_UNWRAP(self);
    HRESULT hr;

    if (d3d9ex_windowed) {
        fdm = NULL;
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    } else {
        if (d3d9ex_force_refresh_rate > 0) {
            log_info(
                "Forcing refresh rate %d -> %d",
                pp->FullScreen_RefreshRateInHz,
                d3d9ex_force_refresh_rate);
            pp->FullScreen_RefreshRateInHz = d3d9ex_force_refresh_rate;
            if (fdm) {
                fdm->RefreshRate = pp->FullScreen_RefreshRateInHz;
            }
        }
    }

    if (d3d9ex_device_adapter >= 0) {
        log_info("Forcing adapter %d -> %d", adapter, d3d9ex_device_adapter);
        adapter = d3d9ex_device_adapter;
    }

    hr = IDirect3D9Ex_CreateDeviceEx(
        real, adapter, type, hwnd, flags, pp, fdm, pdev);

    // TODO stuff

    return hr;
}

static HRESULT STDCALL my_Direct3DCreate9Ex(UINT sdk_ver, IDirect3D9Ex **api)
{
    HRESULT hr;
    IDirect3D9ExVtbl *api_vtbl;
    struct com_proxy *api_proxy;
    IDirect3D9Ex *api_;

    log_info("Direct3DCreate9Ex hook hit");

    hr = real_Direct3DCreate9Ex(sdk_ver, api);
    api_ = *api;
    api_proxy = com_proxy_wrap(api_, sizeof(*api_->lpVtbl));
    api_vtbl = api_proxy->vptr;

    api_vtbl->CreateDeviceEx = my_CreateDeviceEx;

    *api = (IDirect3D9Ex *) api_proxy;

    return hr;
}

static BOOL STDCALL my_EnumDisplayDevicesA(
    const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info, DWORD flags)
{
    BOOL ok;

    ok = real_EnumDisplayDevicesA(dev_name, dev_num, info, flags);

    return ok;
}

void d3d9ex_hook_init(void)
{
    hook_table_apply(
        NULL, "d3d9.dll", d3d9ex_hook_syms, lengthof(d3d9ex_hook_syms));

    hook_table_apply(
        NULL,
        "user32.dll",
        d3d9ex_hook_user32_syms,
        lengthof(d3d9ex_hook_user32_syms));

    log_info("Inserted graphics hooks");
}

void d3d9ex_configure(struct d3d9exhook_config_gfx *gfx_config)
{
    d3d9ex_windowed = gfx_config->windowed;
    d3d9ex_window_framed = gfx_config->framed;
    d3d9ex_window_width = gfx_config->window_width;
    d3d9ex_window_height = gfx_config->window_height;

    d3d9ex_force_refresh_rate = gfx_config->forced_refresh_rate;
    d3d9ex_device_adapter = gfx_config->device_adapter;
}

/* ------------------------------------------------------------------------- */

static void calc_win_size_with_framed(
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

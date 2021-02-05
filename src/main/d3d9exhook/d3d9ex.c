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

static HRESULT STDCALL my_EnumAdapterModes(
    IDirect3D9Ex *self,
    UINT Adapter,
    D3DFORMAT Format,
    UINT Mode,
    D3DDISPLAYMODE *pMode);

static UINT STDCALL my_GetAdapterModeCount(
    IDirect3D9Ex *self,
    UINT Adapter,
    D3DFORMAT Format);

static HRESULT STDCALL my_Direct3DCreate9Ex(UINT sdk_ver, IDirect3D9Ex **api);

static WNDPROC real_WndProc;

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
static bool d3d9ex_confined;
static int32_t d3d9ex_force_refresh_rate = -1;
static int32_t d3d9ex_window_width = -1;
static int32_t d3d9ex_window_height = -1;
static int32_t d3d9ex_window_x = -1;
static int32_t d3d9ex_window_y = -1;
static bool d3d9ex_window_framed;
static int32_t d3d9ex_device_adapter = -1;
static int32_t d3d9ex_force_orientation = -1;
static int32_t d3d9ex_force_screen_res_width = -1;
static int32_t d3d9ex_force_screen_res_height = -1;

static bool check_d3d9ex_force_enum()
{
    if (d3d9ex_force_refresh_rate != -1 &&
        d3d9ex_force_screen_res_width != -1 &&
        d3d9ex_force_screen_res_height != -1) {
        return true;
    }

    return false;
}

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
    if (d3d9ex_windowed) {
        if (d3d9ex_window_framed) {
            /* use a different style */
            dwStyle |= WS_OVERLAPPEDWINDOW;
            /* also show mouse cursor */
            ShowCursor(TRUE);
        } else {
            dwStyle = dwStyle & ~WS_OVERLAPPEDWINDOW;
            ShowCursor(FALSE);
        }
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
    if (d3d9ex_windowed) {
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
        if (d3d9ex_window_x != -1 && d3d9ex_window_y != -1) {
            log_misc(
                "Overriding window position from %dx%d with %dx%d",
                X,
                Y,
                d3d9ex_window_x,
                d3d9ex_window_y);

            X = d3d9ex_window_x;
            Y = d3d9ex_window_y;
        }

        if (d3d9ex_window_framed) {
            /* we have to adjust the window size, because the window needs to
            be a slightly bigger than the rendering resolution (window caption
            and stuff is included in the window size) */

            WINDOWPOS wp;
            calc_win_size_with_framed(hWnd, X, Y, nWidth, nHeight, &wp);
            SetWindowPos(hWnd, 0, wp.x, wp.y, wp.cx, wp.cy, 0);
            X = wp.x;
            Y = wp.y;
            nWidth = wp.cx;
            nHeight = wp.cy;
        }
    }

    BOOL result = real_MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

    return result;
}

static LRESULT gfx_confine(HWND hwnd)
{
    POINT p;
    RECT r;

    log_misc("Confining mouse (ALT-TAB to release)");

    p.x = 0;
    p.y = 0;

    ClientToScreen(hwnd, &p);

    r.left = p.x;
    r.top = p.y;
    r.right = p.x + 100;
    r.bottom = p.y + 100;

    ClipCursor(&r);

    return TRUE;
}

static LRESULT gfx_unconfine(HWND hwnd)
{
    log_misc("Un-confining mouse");

    ClipCursor(NULL);

    return TRUE;
}

static LRESULT CALLBACK
my_WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_SETFOCUS:
            return gfx_confine(hwnd);

        case WM_KILLFOCUS:
            return gfx_unconfine(hwnd);

        default:
            return CallWindowProc(real_WndProc, hwnd, msg, wparam, lparam);
    }
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
    log_assert(self);
    log_assert(pp);
    log_assert(pdev);

    IDirect3D9Ex *real = com_proxy_downcast(self)->real;
    HRESULT hr;

    log_misc(
        "CreateDeviceEx parameters: adapter %d, type %d, hwnd %p, flags %lX, "
        "pdev %p",
        adapter,
        type,
        hwnd,
        flags,
        pdev);

    if (fdm) {
        log_misc(
            "CreateDeviceEx display mode: size %d, width %d, height %d, refresh rate %d, format %d, "
            "scan line ordering %d",
            fdm->Size,
            fdm->Width,
            fdm->Height,
            fdm->RefreshRate,
            fdm->Format,
            fdm->ScanLineOrdering);
    }

    log_misc(
        "D3D9EX presenter parameters: BackBufferWidth %d, BackBufferHeight "
        "%d, BackBufferFormat %d, "
        "BackBufferCount %d, MultiSampleType %d, MultiSampleQuality %ld, SwapEffect %d, "
        "hDeviceWindow %p, Windowed %d, "
        "EnableAutoDepthStencil "
        "%d, AutoDepthStencilFormat %d, Flags %lX, "
        "FullScreen_RefreshRateInHz %d, PresentationInterval %d",
        pp->BackBufferWidth,
        pp->BackBufferHeight,
        pp->BackBufferFormat,
        pp->BackBufferCount,
        pp->MultiSampleType,
        pp->MultiSampleQuality,
        pp->SwapEffect,
        pp->hDeviceWindow,
        pp->Windowed,
        pp->EnableAutoDepthStencil,
        pp->AutoDepthStencilFormat,
        pp->Flags,
        pp->FullScreen_RefreshRateInHz,
        pp->PresentationInterval);

    if (d3d9ex_device_adapter >= 0) {
        log_info("Forcing adapter %d -> %d", adapter, d3d9ex_device_adapter);
        adapter = d3d9ex_device_adapter;
    }

    if (d3d9ex_force_screen_res_width > 0 && d3d9ex_force_screen_res_height > 0) {
        log_info("Overriding screen resolution/back buffer of %dx%d -> %dx%d",
            pp->BackBufferWidth,
            pp->BackBufferHeight,
            d3d9ex_force_screen_res_width,
            d3d9ex_force_screen_res_height);

        pp->BackBufferWidth = d3d9ex_force_screen_res_width;
        pp->BackBufferHeight = d3d9ex_force_screen_res_height;

        if (fdm) {
            fdm->Width = pp->BackBufferWidth;
            fdm->Height = pp->BackBufferHeight;
        }
    }

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

        if (d3d9ex_force_orientation >= DMDO_DEFAULT &&
            d3d9ex_force_orientation <= DMDO_270) {
            D3DADAPTER_IDENTIFIER9 adapter_ident;
            if (IDirect3D9Ex_GetAdapterIdentifier(
                    real, adapter, 0, &adapter_ident) == D3D_OK) {
                // straight outta MSDN
                DEVMODE dm;
                // initialize the DEVMODE structure
                ZeroMemory(&dm, sizeof(dm));
                dm.dmSize = sizeof(dm);

                if (EnumDisplaySettings(
                        adapter_ident.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {

                    int32_t delta =
                        d3d9ex_force_orientation - dm.dmDisplayOrientation;

                    if (delta % 2 != 0) {
                        // swap height and width
                        DWORD dwTemp = dm.dmPelsHeight;
                        dm.dmPelsHeight = dm.dmPelsWidth;
                        dm.dmPelsWidth = dwTemp;
                    }

                    dm.dmDisplayOrientation = d3d9ex_force_orientation;

                    long cd_ret = ChangeDisplaySettingsEx(
                        adapter_ident.DeviceName,
                        &dm,
                        NULL,
                        CDS_FULLSCREEN,
                        NULL);

                    if (cd_ret == DISP_CHANGE_SUCCESSFUL) {
                        log_info(
                            "Overriding rotation suceeded: %s",
                            adapter_ident.DeviceName);
                    } else {
                        log_info(
                            "Overriding rotation failed %s %ld",
                            adapter_ident.DeviceName,
                            cd_ret);
                    }
                }
            }
        }
    }

    hr = IDirect3D9Ex_CreateDeviceEx(
        real, adapter, type, hwnd, flags, pp, fdm, pdev);

    if (SUCCEEDED(hr) && d3d9ex_confined) {
        real_WndProc = (void *) GetWindowLongPtr(hwnd, GWLP_WNDPROC);

        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (uintptr_t) my_WndProc);
    }

    return hr;
}

static HRESULT STDCALL my_EnumAdapterModes(
    IDirect3D9Ex *self,
    UINT Adapter,
    D3DFORMAT Format,
    UINT Mode,
    D3DDISPLAYMODE *pMode)
{
    IDirect3D9Ex *real = com_proxy_downcast(self)->real;
    HRESULT hr;

    if (d3d9ex_device_adapter>= 0) {
        Adapter = d3d9ex_device_adapter;
    }

    hr = IDirect3D9Ex_EnumAdapterModes(real, Adapter, Format, Mode, pMode);

    if (check_d3d9ex_force_enum()) {
        log_info("EnumAdapterModes: Forcing mode");
        pMode->Width = d3d9ex_force_screen_res_width;
        pMode->Height = d3d9ex_force_screen_res_height;
        pMode->RefreshRate = d3d9ex_force_refresh_rate;
    }

    return hr;
}

static UINT STDCALL my_GetAdapterModeCount(
    IDirect3D9Ex *self,
    UINT Adapter,
    D3DFORMAT Format)
{
    IDirect3D9Ex *real = com_proxy_downcast(self)->real;
    UINT res;

    if (d3d9ex_device_adapter>= 0) {
        Adapter = d3d9ex_device_adapter;
    }

    res = IDirect3D9Ex_GetAdapterModeCount(real, Adapter, Format);

    if (check_d3d9ex_force_enum()) {
        log_info("GetAdapterModeCount: Forcing single return");
        res = 1;
    }

    return res;
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
    
    hr = com_proxy_wrap(&api_proxy, api_, sizeof(*api_->lpVtbl));
    
    if (hr != S_OK) {
        log_warning("Wrapping com proxy failed: %08lx", hr);
        return hr;
    }
    
    api_vtbl = api_proxy->vptr;

    api_vtbl->CreateDeviceEx = my_CreateDeviceEx;
    api_vtbl->EnumAdapterModes = my_EnumAdapterModes;
    api_vtbl->GetAdapterModeCount = my_GetAdapterModeCount;

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
    d3d9ex_confined = gfx_config->confined;
    d3d9ex_window_framed = gfx_config->framed;
    d3d9ex_window_width = gfx_config->window_width;
    d3d9ex_window_height = gfx_config->window_height;
    d3d9ex_window_x = gfx_config->window_x;
    d3d9ex_window_y = gfx_config->window_y;

    d3d9ex_force_refresh_rate = gfx_config->forced_refresh_rate;
    d3d9ex_device_adapter = gfx_config->device_adapter;
    d3d9ex_force_orientation = gfx_config->force_orientation;

    d3d9ex_force_screen_res_width = gfx_config->force_screen_res.width;
    d3d9ex_force_screen_res_height = gfx_config->force_screen_res.height;

    if (d3d9ex_force_screen_res_width * d3d9ex_force_screen_res_height > 0) {
        log_warning("Force screen res: Only one, either width or height, is > 0."
            " Force screen res not activate");
    }
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

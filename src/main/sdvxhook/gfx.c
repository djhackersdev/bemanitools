#include <d3d9.h>
#include <windows.h>

#include <stdbool.h>

#include "hook/com-proxy.h"
#include "hook/pe.h"
#include "hook/table.h"

#include "sdvxhook/gfx.h"

#include "util/defs.h"
#include "util/log.h"

static LRESULT CALLBACK
my_WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static HRESULT STDCALL my_CreateDevice(
    IDirect3D9 *self,
    UINT adapter,
    D3DDEVTYPE type,
    HWND hwnd,
    DWORD flags,
    D3DPRESENT_PARAMETERS *pp,
    IDirect3DDevice9 **pdev);
static IDirect3D9 *STDCALL my_Direct3DCreate9(UINT sdk_ver);

static WNDPROC real_WndProc;
static IDirect3D9 *(STDCALL *real_Direct3DCreate9)(UINT sdk_ver);

static const struct hook_symbol gfx_hooks[] = {
    {.name = "Direct3DCreate9",
     .patch = my_Direct3DCreate9,
     .link = (void **) &real_Direct3DCreate9},
};

static bool gfx_confined;
static bool gfx_windowed;

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

static HRESULT STDCALL my_CreateDevice(
    IDirect3D9 *self,
    UINT adapter,
    D3DDEVTYPE type,
    HWND hwnd,
    DWORD flags,
    D3DPRESENT_PARAMETERS *pp,
    IDirect3DDevice9 **pdev)
{
    IDirect3D9 *real = com_proxy_downcast(self)->real;
    HRESULT hr;

    log_misc("IDirect3D9::CreateDevice hook hit");

    if (gfx_windowed) {
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    }

    hr = IDirect3D9_CreateDevice(real, adapter, type, hwnd, flags, pp, pdev);

    if (SUCCEEDED(hr) && gfx_confined) {
        real_WndProc = (void *) GetWindowLongPtr(hwnd, GWLP_WNDPROC);

        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (uintptr_t) my_WndProc);
    }

    return hr;
}

static IDirect3D9 *STDCALL my_Direct3DCreate9(UINT sdk_ver)
{
    IDirect3D9 *api;
    IDirect3D9Vtbl *api_vtbl;
    struct com_proxy *api_proxy;
    HRESULT hr;

    log_info("Direct3DCreate9 hook hit");

    api = real_Direct3DCreate9(sdk_ver);

    hr = com_proxy_wrap(&api_proxy, api, sizeof(*api->lpVtbl));

    if (hr != S_OK) {
        log_fatal("Wrapping com proxy failed: %08lx", hr);
    }

    api_vtbl = api_proxy->vptr;

    api_vtbl->CreateDevice = my_CreateDevice;

    return (IDirect3D9 *) api_proxy;
}

void gfx_init(void)
{
    hook_table_apply(NULL, "d3d9.dll", gfx_hooks, lengthof(gfx_hooks));
    log_info("Inserted graphics hooks");
}

void gfx_set_confined(void)
{
    gfx_confined = true;
}

void gfx_set_windowed(void)
{
    gfx_windowed = true;
}

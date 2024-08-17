#include <d3d9.h>
#include <windows.h>

#include <stdbool.h>

#include "iface-core/log.h"

#include "hook/com-proxy.h"
#include "hook/pe.h"
#include "hook/table.h"

#include "sdvxhook/gfx.h"

#include "util/defs.h"

static HRESULT STDCALL my_CreateDevice(
    IDirect3D9 *self,
    UINT adapter,
    D3DDEVTYPE type,
    HWND hwnd,
    DWORD flags,
    D3DPRESENT_PARAMETERS *pp,
    IDirect3DDevice9 **pdev);
static IDirect3D9 *STDCALL my_Direct3DCreate9(UINT sdk_ver);

static IDirect3D9 *(STDCALL *real_Direct3DCreate9)(UINT sdk_ver);

static const struct hook_symbol gfx_hook_syms[] = {
    {.name = "Direct3DCreate9",
     .patch = my_Direct3DCreate9,
     .link = (void **) &real_Direct3DCreate9},
};

static bool gfx_windowed;

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

    return hr;
}

static IDirect3D9 *STDCALL my_Direct3DCreate9(UINT sdk_ver)
{
    IDirect3D9 *api;
    IDirect3D9Vtbl *api_vtbl;
    struct com_proxy *api_proxy;
    HRESULT res;

    log_info("Direct3DCreate9 hook hit");

    api = real_Direct3DCreate9(sdk_ver);
    res = com_proxy_wrap(&api_proxy, api, sizeof(*api->lpVtbl));

    if (res != S_OK) {
        log_fatal("Wrapping com proxy failed: 0x%ld", res);
    }

    api_vtbl = api_proxy->vptr;

    api_vtbl->CreateDevice = my_CreateDevice;

    return (IDirect3D9 *) api_proxy;
}

void gfx_init(void)
{
    hook_table_apply(NULL, "d3d9.dll", gfx_hook_syms, lengthof(gfx_hook_syms));

    log_info("Inserted graphics hooks");
}

void gfx_set_windowed(void)
{
    gfx_windowed = true;
}

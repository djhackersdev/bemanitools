#include <windows.h>
#include <d3d9.h>

#include <stdbool.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"

static HRESULT STDCALL my_CreateDevice(
        IDirect3D9 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd,
        DWORD flags, D3DPRESENT_PARAMETERS *pp, IDirect3DDevice9 **pdev);

static IDirect3D9 *STDCALL my_Direct3DCreate9(UINT sdk_ver);

static IDirect3D9 * (STDCALL *real_Direct3DCreate9)(
        UINT sdk_ver);

static bool gfx_windowed;

static const struct hook_symbol gfx_d3d9_hook_syms[] = {
    {
        .name   = "Direct3DCreate9",
        .patch  = my_Direct3DCreate9,
        .link   = (void **) &real_Direct3DCreate9,
    },
};

static HRESULT STDCALL my_CreateDevice(
        IDirect3D9 *self,
        UINT adapter,
        D3DDEVTYPE type,
        HWND hwnd,
        DWORD flags,
        D3DPRESENT_PARAMETERS *pp,
        IDirect3DDevice9 **pdev)
{
    IDirect3D9 *real;

    real = COM_PROXY_UNWRAP(self);

    if (gfx_windowed) {
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    }

    return IDirect3D9_CreateDevice(real, adapter, type, hwnd, flags, pp, pdev);
}

static IDirect3D9 *STDCALL my_Direct3DCreate9(UINT sdk_ver)
{
    IDirect3D9 *api;
    IDirect3D9Vtbl *api_vtbl;
    struct com_proxy *api_proxy;

    log_info("Direct3DCreate9 hook hit");

    api = real_Direct3DCreate9(sdk_ver);
    api_proxy = com_proxy_wrap(api, sizeof(*api->lpVtbl));
    api_vtbl = api_proxy->vptr;

    api_vtbl->CreateDevice = my_CreateDevice;

    return (IDirect3D9 *) api_proxy;
}

void gfx_insert_hooks(HMODULE target)
{
    hook_table_apply(
            target,
            "d3d9.dll",
            gfx_d3d9_hook_syms,
            lengthof(gfx_d3d9_hook_syms));

    log_info("Inserted graphics hooks");
}

bool gfx_get_windowed(void)
{
    return gfx_windowed;
}

void gfx_set_windowed(void)
{
    gfx_windowed = true;
}


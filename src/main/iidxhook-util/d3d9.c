#define LOG_MODULE "d3d9-hook"

#include <windows.h>
#include <d3d9.h>
#include <d3dx9core.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "iidxhook-util/d3d9.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

/* ------------------------------------------------------------------------- */

struct d3d8_vertex {
    float x;
    float y;
    float z;
    uint32_t color;
    uint32_t unknown;
    float tu;
    float tv;
};

/* ------------------------------------------------------------------------- */

/* Device IDs recognised by IIDX <20. Hex chars must be capital letters.

   7146: RV515      [Radeon X1300]
   95C5: RV620 LE   [Radeon HD 3450]

   VEN is always 1002 (ATi) */

static HWND STDCALL my_CreateWindowExA(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam);

static BOOL STDCALL my_GetClientRect(HWND hWnd, LPRECT lpRect);

static HRESULT STDCALL my_CreateDevice(
        IDirect3D9 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd,
        DWORD flags, D3DPRESENT_PARAMETERS *pp, IDirect3DDevice9 **pdev);

static IDirect3D9 *STDCALL my_Direct3DCreate9(UINT sdk_ver);

static BOOL STDCALL my_EnumDisplayDevicesA(
        const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info,
        DWORD flags);

static HRESULT STDCALL my_CreateTexture(IDirect3DDevice9* self, UINT Width,
        UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
        IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);

static HRESULT STDCALL my_SetRenderState(IDirect3DDevice9* self,
        D3DRENDERSTATETYPE State, DWORD Value);

static HRESULT STDCALL my_DrawPrimitiveUP(
        IDirect3DDevice9* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride);

static HRESULT STDCALL my_Present(IDirect3DDevice9* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);

static HRESULT STDCALL my_BeginScene(IDirect3DDevice9* self);

static HRESULT STDCALL my_EndScene(IDirect3DDevice9* self);

static void calc_win_size_with_framed(HWND hwnd, DWORD x, DWORD y,
        DWORD width, DWORD height, LPWINDOWPOS wp);

static HWND (STDCALL *real_CreateWindowExA)(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam);

static BOOL (STDCALL* real_GetClientRect)(HWND hWnd, LPRECT lpRect);

static IDirect3D9 * (STDCALL *real_Direct3DCreate9)(
        UINT sdk_ver);

static BOOL (STDCALL *real_EnumDisplayDevicesA)(
        const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info,
        DWORD flags);

static HRESULT (STDCALL *real_CreateTexture)(IDirect3DDevice9* self, UINT Width,
        UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
        IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);

static HRESULT (STDCALL* real_SetRenderState)(IDirect3DDevice9* self,
        D3DRENDERSTATETYPE State, DWORD Value);

static HRESULT (STDCALL *real_DrawPrimitiveUP)(
        IDirect3DDevice9* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride);

static HRESULT (STDCALL* real_Present)(IDirect3DDevice9* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);

static HRESULT (STDCALL* real_BeginScene)(IDirect3DDevice9* self);

static HRESULT (STDCALL* real_EndScene)(IDirect3DDevice9* self);

static HRESULT (STDCALL* real_GetRenderTarget)(IDirect3DDevice9* self, DWORD RenderTargetIndex, 
        IDirect3DSurface9 **ppRenderTarget);

static HRESULT (STDCALL* real_SetRenderTarget)(IDirect3DDevice9* self, DWORD RenderTargetIndex, 
        IDirect3DSurface9 *pRenderTarget);

static HRESULT (STDCALL* real_StretchRect)(IDirect3DDevice9* self, IDirect3DSurface9 *pSourceSurface, 
        const RECT *pSourceRect, IDirect3DSurface9 *pDestSurface, const RECT *pDestRect, D3DTEXTUREFILTERTYPE Filter);

typedef HRESULT WINAPI (*func_D3DXCreateFontA)(struct IDirect3DDevice9 *device,
        INT height, UINT width, UINT weight, UINT miplevels, BOOL italic,
        DWORD charset, DWORD precision, DWORD quality, DWORD pitchandfamily,
        const char *facename, struct ID3DXFont **font);

/* ------------------------------------------------------------------------- */

static char d3d9_pci_id[32];
static bool d3d9_windowed;
static int32_t d3d9_window_width = -1;
static int32_t d3d9_window_height = -1;
static bool d3d9_window_framed;
static d3d9_monitor_check_result_callback_t d3d9_monitor_check_cb;
static bool d3d9_fix_iidx_12_fog = false;
static bool d3d9_fix_iidx_13_lighting = false;
static bool d3d9_nvidia_fix;
static bool d3d9_bg_video_fix;
static float d3d9_frame_rate_limit = 0.0f;
static uint16_t d3d9_scale_back_buffer_width;
static uint16_t d3d9_scale_back_buffer_height;

static D3DTEXTUREFILTERTYPE d3d9_scale_back_buffer_filter;
static IDirect3DSurface9* d3d9_original_back_buffer;
static IDirect3DSurface9* d3d9_scale_intermediate_render_target;
static uint16_t d3d9_original_back_buffer_width;
static uint16_t d3d9_original_back_buffer_height;

static void execute_monitor_check(IDirect3DDevice9* api,
    IDirect3DDevice9Vtbl* api_vtbl, IDirect3DDevice9* pdev);

/* ------------------------------------------------------------------------- */

static const struct hook_symbol d3d9_hook_syms[] = {
    {
        .name   = "Direct3DCreate9",
        .patch  = my_Direct3DCreate9,
        .link   = (void **) &real_Direct3DCreate9
    },
};

static const struct hook_symbol d3d9_hook_user32_syms[] = {
    {
        .name   = "EnumDisplayDevicesA",
        .patch  = my_EnumDisplayDevicesA,
        .link   = (void **) &real_EnumDisplayDevicesA
    },
    {
        .name   = "CreateWindowExA",
        .patch  = my_CreateWindowExA,
        .link  = (void **) &real_CreateWindowExA
    },
    {
        .name   = "GetClientRect",
        .patch  = my_GetClientRect,
        .link  = (void **) &real_GetClientRect
    },
};

/* ------------------------------------------------------------------------- */

static bool float_equal(float a, float b, float eps)
{
    return fabs(a - b) < eps;
}

static HWND STDCALL my_CreateWindowExA(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam)
{
    if (d3d9_windowed && d3d9_window_framed) {
        /* use a different style */
        dwStyle |= WS_OVERLAPPEDWINDOW;
        /* also show mouse cursor */
        ShowCursor(TRUE);
    }

    if (d3d9_window_width != -1 && d3d9_window_height != -1) {
        log_misc("Overriding window size from %dx%d with %dx%d", nWidth,
            nHeight, d3d9_window_width, d3d9_window_height);

        nWidth = d3d9_window_width;
        nHeight = d3d9_window_height;
    }

    HWND hwnd = real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName,
            dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance,
            lpParam);

    if (hwnd == INVALID_HANDLE_VALUE) {
        return hwnd;
    }

    if (d3d9_windowed && d3d9_window_framed) {
        /* we have to adjust the window size, because the window needs to be a
           slightly bigger than the rendering resolution (window caption and
           stuff is included in the window size) */
        WINDOWPOS wp;
        calc_win_size_with_framed(hwnd, X, Y, nWidth, nHeight, &wp);
        SetWindowPos(hwnd, 0, wp.x, wp.y, wp.cx, wp.cy, 0);
        X = wp.x;
        Y = wp.y;
        nWidth = wp.cx;
        nHeight = wp.cy;
    }

    return hwnd;
}

static BOOL STDCALL my_GetClientRect(HWND hWnd, LPRECT lpRect)
{
    /* IIDX 9-13 (at least) use this call to get the size of rectangle for setting the viewport size... */

    /* Always use the original requested back buffer size. If scaling is active, using the scaled
       values leads to (3D) scenes getting render to a viewport with incorrect sized. */
    lpRect->left = 0;
    lpRect->top = 0;
    lpRect->right = d3d9_original_back_buffer_width;
    lpRect->bottom = d3d9_original_back_buffer_height;

    return TRUE;
}

static HRESULT STDCALL my_CreateDevice(
    IDirect3D9 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd, DWORD flags,
    D3DPRESENT_PARAMETERS *pp, IDirect3DDevice9 **pdev)
{
    IDirect3D9 *real = COM_PROXY_UNWRAP(self);
    IDirect3DDevice9* api;
    IDirect3DDevice9Vtbl *api_vtbl;
    struct com_proxy *api_proxy;
    HRESULT hr;
    char* error;

    log_misc("CreateDevice parameters: adapter %d, type %d, hwnd %p, flags %lX, pdev %p", 
            adapter, type, hwnd, flags, pdev);

    log_misc("D3D9 presenter parameters: BackBufferWidth %d, BackBufferHeight %d, BackBufferFormat %d, "
        "BackBufferCount %d, MultiSampleType %d, SwapEffect %d, hDeviceWindow %p, Windowed %d, EnableAutoDepthStencil "
        "%d, AutoDepthStencilFormat %d, Flags %lX, FullScreen_RefreshRateInHz %d",
            pp->BackBufferWidth, pp->BackBufferHeight, pp->BackBufferFormat, pp->BackBufferCount, pp->MultiSampleType,
            pp->SwapEffect, pp->hDeviceWindow, pp->Windowed, pp->EnableAutoDepthStencil, pp->AutoDepthStencilFormat,
            pp->Flags, pp->FullScreen_RefreshRateInHz);

    /* Fix a long-standing bug in IIDX */
    if (flags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) {
        flags &= ~D3DCREATE_PUREDEVICE;
    }

    if (d3d9_windowed) {
        log_misc("Window mode");

        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    }

    /* Store these so we can apply scaling further down */
    d3d9_original_back_buffer_width = pp->BackBufferWidth;
    d3d9_original_back_buffer_height = pp->BackBufferHeight;

    if (d3d9_scale_back_buffer_width > 0 && d3d9_scale_back_buffer_height > 0) {
        log_misc("Scale back buffer from %dx%d -> %dx%d", d3d9_original_back_buffer_width, 
            d3d9_original_back_buffer_height, d3d9_scale_back_buffer_width, d3d9_scale_back_buffer_height);

        pp->BackBufferWidth = d3d9_scale_back_buffer_width;
        pp->BackBufferHeight = d3d9_scale_back_buffer_height;
    }

    /* Same fix as on D3D8, orz...
       If we don't do this, some games one certain platforms (e.g. iidx 14/15 on Windows 10).
       CreateDevice fails with an "invalid call" on either fullscreen or windowed or even both. 
       Also, further reports about textures with green glowing borders are gone as well when applying this */
    D3DDISPLAYMODE mode;
    IDirect3D9_GetAdapterDisplayMode(real, adapter, &mode);
    pp->BackBufferFormat = mode.Format;

    hr = IDirect3D9_CreateDevice(real, adapter, type, hwnd, flags, pp, pdev);

    if (hr != S_OK) {
        switch (hr) {
            case D3DERR_DEVICELOST:
                error = "device lost";
                break;

            case D3DERR_INVALIDCALL:
                error = "invalid call";
                break;

            case D3DERR_NOTAVAILABLE:
                error = "not available";
                break;

            case D3DERR_OUTOFVIDEOMEMORY:
                error = "out of video memory";
                break;

            default:
                error = "unknown";
                break;
        }

        log_warning("Creating D3D9 device failed: %lX, %s", hr, error);

        return hr;
    }

    api = *pdev;
    api_proxy = com_proxy_wrap(api, sizeof(*api->lpVtbl));
    api_vtbl = api_proxy->vptr;

    real_CreateTexture = api_vtbl->CreateTexture;
    api_vtbl->CreateTexture = my_CreateTexture;

    real_DrawPrimitiveUP = api_vtbl->DrawPrimitiveUP;
    api_vtbl->DrawPrimitiveUP = my_DrawPrimitiveUP;

    real_SetRenderState = api_vtbl->SetRenderState;
    api_vtbl->SetRenderState = my_SetRenderState;

    real_Present = api_vtbl->Present;
    api_vtbl->Present = my_Present;

    real_BeginScene = api_vtbl->BeginScene;
    api_vtbl->BeginScene = my_BeginScene;

    real_EndScene = api_vtbl->EndScene;
    api_vtbl->EndScene = my_EndScene;

    real_SetRenderTarget = api_vtbl->SetRenderTarget;
    real_GetRenderTarget = api_vtbl->GetRenderTarget;
    real_StretchRect = api_vtbl->StretchRect;

    *pdev = (IDirect3DDevice9*) api_proxy;

    if (d3d9_scale_back_buffer_width > 0 && d3d9_scale_back_buffer_height > 0) {
        hr = api_vtbl->CreateRenderTarget(*pdev, d3d9_scale_back_buffer_width, d3d9_scale_back_buffer_height,
                pp->BackBufferFormat, pp->MultiSampleType, 0, false, &d3d9_scale_intermediate_render_target, NULL);
        
        if (hr != D3D_OK) {
            log_warning("Creating intermediate render target for scaling back buffer failed: %ld", hr);
            return hr;
        }
    }

    if (d3d9_monitor_check_cb) {
        execute_monitor_check(api, api_vtbl, *pdev);
    }

    /* Avoid returning our scaled values because the application might use them, e.g. calculate
       sprite positions. */
    pp->BackBufferWidth = d3d9_original_back_buffer_width;
    pp->BackBufferHeight = d3d9_original_back_buffer_height;

    return hr;
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

static BOOL STDCALL my_EnumDisplayDevicesA(
        const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info,
        DWORD flags)
{
    BOOL ok;

    ok = real_EnumDisplayDevicesA(dev_name, dev_num, info, flags);

    if (ok && d3d9_pci_id[0] != '\0') {
        /* Apparently Konami didn't read the "Not Used" message in the MSDN
           docs for DISPLAY_DEVICE */
        log_misc("Replacing device ID %s with %s",
                info->DeviceID, d3d9_pci_id);

        str_cpy(info->DeviceID, sizeof(info->DeviceID), d3d9_pci_id);
    }

    return ok;
}

static HRESULT STDCALL my_CreateTexture(IDirect3DDevice9* self, UINT Width,
        UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
        IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
    /* Fix non ATI cards not working on Gold (and DJT?). There is a check that
       creates textures when starting the game. This check fails which results
       in a message box being shown mentioning something about an aep-lib error.
       Seems like a combination of parameters is not supported by
       non ATI cards and throws an error
       (Fix taken from old Gold cracks by ahnada and tau)
    */
    if (d3d9_nvidia_fix && Width == 256 && Height == 256 && Levels == 1 &&
            Usage == 1 && Format == 25 && Pool == 0 && pSharedHandle == 0) {
        /* log_misc("CreateTexture: Patching texture format..."); */
        Format = 21;
    }

    return real_CreateTexture(self, Width, Height, Levels, Usage, Format, Pool,
        ppTexture, pSharedHandle);
}

static HRESULT STDCALL my_SetRenderState(IDirect3DDevice9* self,
        D3DRENDERSTATETYPE State, DWORD Value)
{
    if (d3d9_fix_iidx_12_fog) {
        if (State == D3DRS_FOGENABLE) {
            Value = FALSE;
        }
    }

    if (d3d9_fix_iidx_13_lighting) {
        if (State == D3DRS_LIGHTING) {
            Value = FALSE;
        }
    }

    return real_SetRenderState(self, State, Value);
}

static HRESULT STDCALL my_DrawPrimitiveUP(
        IDirect3DDevice9* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride)
{
    /* same code taken from the d3d8 module. but, this just fixes the quad
       seam issue as there are no reports of streched bg videos */
    if (    d3d9_bg_video_fix && primitive_type == 6 && primitive_count == 2 &&
            stride == 28) {

        struct d3d8_vertex* vertices = (struct d3d8_vertex*) data;

        /*
        log_info("Video Tex: %f/%f %f/%f %f/%f %f/%f",
            vertices[0].x, vertices[0].y,
            vertices[1].x, vertices[1].y,
            vertices[2].x, vertices[2].y,
            vertices[3].x, vertices[3].y);
        */

        /* Fix full screen background videos (e.g. DistorteD intro sequence) */
        if (    vertices[0].x >= 0.0f && vertices[0].x < 1.0f &&
                vertices[0].y >= 0.0f && vertices[0].y < 1.0f &&
                vertices[1].x > 639.0f && vertices[1].x < 641.0f &&
                vertices[1].y >= 0.0f && vertices[1].y < 1.0f &&
                vertices[2].x > 639.0f && vertices[2].x < 641.0f &&
                vertices[2].y > 479.0f && vertices[2].y < 481.0f &&
                vertices[3].x >= 0.0f && vertices[3].x < 1.0f &&
                vertices[3].y > 479.0f && vertices[3].y < 481.0f) {
            /* fix UVs
               1.0f / 640 or 480 fixes the diagonal seam connecting the two
               triangles which is visible on some GPUs (why? idk)
            */
            vertices[0].tu = 0.0f + 1.0f / 640;
            vertices[0].tv = 1.0f;
            vertices[1].tu = 1.0f;
            vertices[1].tv = 1.0f;
            vertices[2].tu = 1.0f;
            vertices[2].tv = 0.0f + 1.0f / 480;
            vertices[3].tu = 0.0f + 1.0f / 640;
            vertices[3].tv = 0.0f + 1.0f / 480;
        } else

        /* another identifier, because there are other textures with 512x512 size
           make sure we got the bg video only to not mess up anything else */
        /* different versions have different themes and position the bg video
           on slightly different positions (good job...) */
        if (    /* single */
                ((vertices[0].x >= 164.0f && vertices[0].x <= 168.0f &&
                float_equal(vertices[0].y, 0.0f, 0.1f)) &&
                (vertices[1].x >= 472.0f && vertices[1].x <= 476.0f &&
                float_equal(vertices[1].y, 0.0f, 0.1f)) &&
                (vertices[2].x >= 472.0f && vertices[2].x <= 476.0f &&
                float_equal(vertices[2].y, 416.0f, 0.1f)) &&
                (vertices[3].x >= 164.0f && vertices[3].x <= 168.0f &&
                float_equal(vertices[3].y, 416.0f, 0.1f))) ||
                /* double top left */
                ((float_equal(vertices[0].x, 6.0f, 0.1f) &&
                vertices[0].y >= 24.0f && vertices[0].y <= 28.0f) &&
                (float_equal(vertices[1].x, 147.0f, 0.1f) &&
                vertices[1].y >= 24.0f && vertices[1].y <= 28.0f) &&
                (float_equal(vertices[2].x, 147.0f, 0.1f) &&
                vertices[2].y >= 212.0f && vertices[2].y <= 216.0f) &&
                (float_equal(vertices[3].x, 6.0f, 0.1f) &&
                vertices[3].y >= 212.0f && vertices[3].y <= 216.0f)) ||
                /* double bottom left */
                ((float_equal(vertices[0].x, 6.0f, 0.1f) &&
                vertices[0].y >= 216.0f && vertices[0].y <= 220.0f) &&
                (float_equal(vertices[1].x, 147.0f, 0.1f) &&
                vertices[1].y >= 216.0f && vertices[1].y <= 220.0f) &&
                (float_equal(vertices[2].x, 147.0f, 0.1) &&
                vertices[2].y >= 404.0f && vertices[2].y <= 408.0f) &&
                (float_equal(vertices[3].x, 6.0f, 0.1f) &&
                vertices[3].y >= 404.0f && vertices[3].y <= 408.0f)) ||
                /* double top right */
                ((vertices[0].x >= 493.0f && vertices[0].x <= 494.0f &&
                vertices[0].y >= 24.0f && vertices[0].y <= 28.0f) &&
                (vertices[1].x >= 634.0f && vertices[1].x <= 635.0f &&
                vertices[1].y >= 24.0f && vertices[1].y <= 28.0f) &&
                (vertices[2].x >= 634.0f && vertices[2].x <= 635.0f &&
                vertices[2].y >= 212.0f && vertices[2].y <= 216.0f) &&
                (vertices[3].x >= 493.0f && vertices[3].x <= 494.0f &&
                vertices[3].y >= 212.0f && vertices[3].y <= 216.0f)) ||
                /* double bottom right */
                ((vertices[0].x >= 493.0f && vertices[0].x <= 494.0f &&
                vertices[0].y >= 216.0f && vertices[0].y <= 220.0f) &&
                (vertices[1].x >= 634.0f && vertices[1].x <= 635.0f &&
                vertices[1].y >= 216.0f  && vertices[1].y <= 220.0f) &&
                (vertices[2].x >= 634.0f && vertices[2].x <= 635.0f &&
                vertices[2].y >= 404.0f && vertices[2].y <= 408.0f) &&
                (vertices[3].x >= 493.0f && vertices[3].x <= 494.0f &&
                vertices[3].y >= 404.0f && vertices[3].y <= 408.0f)))
        {
            /* fix UVs
               1.0f / 512 fixes the diagonal seam connecting the two triangles
               which is visible on some GPUs (why? idk)
            */
            vertices[0].tu = 0.0f + 1.0f / 512;
            vertices[0].tv = 1.0f;
            vertices[1].tu = 1.0f;
            vertices[1].tv = 1.0f;
            vertices[2].tu = 1.0f;
            vertices[2].tv = 0.0f + 1.0f / 512;
            vertices[3].tu = 0.0f + 1.0f / 512;
            vertices[3].tv = 0.0f + 1.0f / 512;
        }

    }

    return real_DrawPrimitiveUP(self, primitive_type, primitive_count,
                data, stride);
}

static HRESULT STDCALL my_Present(IDirect3DDevice9* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
    static uint64_t current_time = 0;

    HRESULT hr = real_Present(self, pSourceRect, pDestRect, hDestWindowOverride,
        pDirtyRegion);

    if (d3d9_frame_rate_limit > 0.0f) {
        if (current_time == 0) {
            current_time = time_get_counter();
        } else {
            uint64_t frame_time = 1000000 / d3d9_frame_rate_limit;

            uint64_t dt = time_get_elapsed_us(
                time_get_counter() - current_time);

            while (dt < frame_time) {
                /* waste some cpu time by polling
                   because we can't sleep for X us */
                dt = time_get_elapsed_us(time_get_counter() - current_time);
            }

            current_time = time_get_counter();
        }
    }

    return hr;
}

static HRESULT STDCALL my_BeginScene(IDirect3DDevice9* self)
{
    HRESULT res;

    if (d3d9_scale_back_buffer_width > 0 && d3d9_scale_back_buffer_height > 0) {
        res = real_GetRenderTarget(self, 0, &d3d9_original_back_buffer);

        if (res != D3D_OK) {
            log_warning("Getting back buffer render target failed: %ld", res);
            return res;
        }

        res = real_SetRenderTarget(self, 0, d3d9_scale_intermediate_render_target);

        if (res != D3D_OK) {
            log_warning("Setting intermediate render target failed: %ld", res);
            return res;
        }
    }

    return real_BeginScene(self);
}

static HRESULT STDCALL my_EndScene(IDirect3DDevice9* self)
{
    HRESULT res = real_EndScene(self);

    if (res == D3D_OK) {
        if (d3d9_scale_back_buffer_width > 0 && d3d9_scale_back_buffer_height > 0) {
            res = real_SetRenderTarget(self, 0, d3d9_original_back_buffer);

            if (res != D3D_OK) {
                log_warning("Setting back back buffer render target failed: %ld", res);
                return res;
            }
            
            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = d3d9_original_back_buffer_width;
            rect.bottom = d3d9_original_back_buffer_height;

            /* Must be called outside of begin-end scene block */
            res = real_StretchRect(self, d3d9_scale_intermediate_render_target, &rect, d3d9_original_back_buffer, NULL, 
                d3d9_scale_back_buffer_filter);

            if (res != D3D_OK) {
                log_warning("Stretching immediate render target to back buffer failed: %ld", res);
                return res;
            }
        }
    }
    
    return res;
}

void d3d9_hook_init(void)
{
    hook_table_apply(
            NULL,
            "d3d9.dll",
            d3d9_hook_syms,
            lengthof(d3d9_hook_syms));

    hook_table_apply(
            NULL,
            "user32.dll",
            d3d9_hook_user32_syms,
            lengthof(d3d9_hook_user32_syms));

    log_info("Inserted graphics hooks");
}

void d3d9_set_windowed(bool framed, int32_t width, int32_t height)
{
    d3d9_windowed = true;
    d3d9_window_framed = framed;
    d3d9_window_width = width;
    d3d9_window_height = height;
}

void d3d9_set_pci_id(uint16_t vid, uint16_t pid)
{
    str_format(d3d9_pci_id, sizeof(d3d9_pci_id), "PCI\\VEN_%04X&DEV_%04X",
            vid, pid);
}

void d3d9_set_frame_rate_limit(float limit)
{
    if (limit < 0.0f) {
        limit = 0.0f;
    }

    log_info("Limiting rendering frame rate to %f frames", limit);
    d3d9_frame_rate_limit = limit;
}

void d3d9_enable_monitor_check(d3d9_monitor_check_result_callback_t cb)
{
    log_assert(cb);

    d3d9_monitor_check_cb = cb;
    log_info("Enabled monitor check");
}

void d3d9_iidx_fix_stretched_bg_videos(void)
{
    /* Same as the seam fix */
    d3d9_bg_video_fix = true;
    log_info("Fixing UVs of stretched BG videos");
}

void d3d9_iidx_fix_12_song_select_bg(void)
{
    d3d9_fix_iidx_12_fog = true;
    log_info("Fixing Happy Sky bugged music select 3D background");
}

void d3d9_iidx_fix_13_song_select_bg(void)
{
    d3d9_fix_iidx_13_lighting = true;
    log_info("Fixing DistorteD music select 3D background");
}

void d3d9_enable_nvidia_fix(void)
{
    d3d9_nvidia_fix = true;
    log_info("Enabled NVIDIA fix");
}

void d3d9_bg_video_seams_fix(void)
{
    d3d9_bg_video_fix = true;
    log_info("Enabled BG video seam fix");
}

void d3d9_scale_back_buffer(uint16_t width, uint16_t height, enum d3d9_back_buffer_scale_filter filter)
{
    d3d9_scale_back_buffer_width = width;
    d3d9_scale_back_buffer_height = height;

    switch (filter) {
        case D3D9_BACK_BUFFER_SCALE_FILTER_NONE:
            d3d9_scale_back_buffer_filter = D3DTEXF_NONE;
            break;

        case D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR:
            d3d9_scale_back_buffer_filter = D3DTEXF_LINEAR;
            break;

        case D3D9_BACK_BUFFER_SCALE_FILTER_POINT:
            d3d9_scale_back_buffer_filter = D3DTEXF_POINT;
            break;

        default:
            log_fatal("Illegal state");
            break;
    }

    if (d3d9_scale_back_buffer_width > 0 && d3d9_scale_back_buffer_height > 0) {
        log_info("Enable scaling of back buffer, target size %dx%d, filter %d", width, height, filter);
    } else {
        d3d9_scale_back_buffer_width = 0;
        d3d9_scale_back_buffer_height = 0;
    }
}

/* ------------------------------------------------------------------------- */

static void execute_monitor_check(IDirect3DDevice9* api,
        IDirect3DDevice9Vtbl* api_vtbl, IDirect3DDevice9* pdev)
{
    log_info("Running monitor check...");

    /* All games include the _24 version anyway, so leave that hardcoded here */
    HMODULE d3d9 = GetModuleHandleA("d3dx9_24.dll");

    if (d3d9 == NULL) {
        log_fatal("monitor check: failed to load d3dx9_24.dll");
    }

    func_D3DXCreateFontA d3dxCreateFontA =
        (func_D3DXCreateFontA) GetProcAddress(d3d9, "D3DXCreateFontA");

    if (d3dxCreateFontA == NULL) {
        log_fatal("monitor check: failed to find function D3DXCreateFontA");
    }

    RECT fRectangle;
    ID3DXFont* font = NULL;
    struct com_proxy *font_api_proxy;
    ID3DXFontVtbl *font_api_vtbl;

    HRESULT hr = d3dxCreateFontA(api, 22, 0, FW_NORMAL, 1, false,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE,
        "Arial", &font);

    font_api_proxy = com_proxy_wrap(font, sizeof(*font->lpVtbl));
    font_api_vtbl = font_api_proxy->vptr;

    font = (ID3DXFont*) font_api_proxy;
    SetRect(&fRectangle, 20, 20, 640, 480);

    if (hr != S_OK) {
        log_fatal("monitor check: Creating font failed");
    }

    if (font == NULL) {
        log_fatal("monitor check: Loading font failed");
    }

    const uint32_t max_iterations = 60 * 30;
    const uint32_t skip_frames = 60 * 1;

    uint64_t accu_us = 0;

    double result = 0;
    uint32_t iterations = 0;

    char text_buffer[256];

    while (iterations < max_iterations) {
        sprintf(text_buffer,
            "Monitor check...\n"
            "Elapsed iterations: %d/%d\n"
            "Refresh rate: %f",
            iterations + 1, max_iterations, result);

        iterations++;
        uint64_t start = time_get_counter();

        api_vtbl->Clear(pdev, 0, NULL, D3DCLEAR_TARGET,
            D3DCOLOR_XRGB(0, 0, 0), 0.0f, 0);
        api_vtbl->BeginScene(pdev);

        font_api_vtbl->DrawTextA(font, NULL, text_buffer, -1, &fRectangle,
            DT_LEFT, D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF));

        api_vtbl->EndScene(pdev);
        api_vtbl->Present(pdev, NULL, NULL, NULL, NULL);

        /* Skip the first inaccurate values */
        if (iterations > skip_frames) {
            accu_us += time_get_elapsed_us(time_get_counter() - start);

            result = ((double) (iterations - skip_frames)) /
                (accu_us / 1000.0 / 1000.0);
        }
    }

    log_info("Monitor check done (total iterations %d), refesh rate: %f hz",
        iterations, result);
    
    /* Sanity check to ensure people notice that their current refresh rate
       is way off. */
    if (result < 55 || result > 65) {
        log_warning("Monitor check result (%f hz) is not even near the "
            "intended refresh rate of 60 hz. Fix your setup to ensure a " 
            "constant and as close to as possible 60 hz refresh rate.", result);
    }

    /* Leave results of monitor check on screen for a moment */
    Sleep(2000);

    log_assert(d3d9_monitor_check_cb);

    d3d9_monitor_check_cb(result);
}

static void calc_win_size_with_framed(HWND hwnd, DWORD x, DWORD y, DWORD width,
        DWORD height, LPWINDOWPOS wp)
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


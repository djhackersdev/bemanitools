#define LOG_MODULE "d3d8-hook"

#include <windows.h>
#include <d3d8.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "iidxhook-util/d3d8.h"

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

static HWND STDCALL my_CreateWindowExA(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam);

static HRESULT STDCALL my_CreateDevice(
        IDirect3D8 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd,
        DWORD flags, D3DPRESENT_PARAMETERS *pp, IDirect3DDevice8 **pdev);

static HRESULT STDCALL my_SetRenderState(IDirect3DDevice8* self,
        D3DRENDERSTATETYPE State, DWORD Value);

static HRESULT STDCALL my_DrawPrimitiveUP(
        IDirect3DDevice8* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride);

static HRESULT STDCALL my_Present(IDirect3DDevice8* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);

static IDirect3D8* STDCALL my_Direct3DCreate8(UINT sdk_ver);

static void execute_monitor_check(IDirect3DDevice8Vtbl* api_vtbl,
        IDirect3DDevice8* pdev);

static void calc_win_size_with_framed(HWND hwnd, DWORD x, DWORD y,
        DWORD width, DWORD height, LPWINDOWPOS wp);

/* ------------------------------------------------------------------------- */

static bool gfx_windowed = false;
static int32_t gfx_window_width = -1;
static int32_t gfx_window_height = -1;
static bool gfx_window_framed = false;
static float gfx_frame_rate_limit = 0.0f;
static bool gfx_fix_stretched_bg_videos = false;
static bool gfx_fix_iidx_12_fog = false;
static bool gfx_fix_iidx_13_lighting = false;
static d3d8_monitor_check_result_callback_t gfx_monitor_check_cb = NULL;

/* ------------------------------------------------------------------------- */

static IDirect3D8* (STDCALL *real_Direct3DCreate8)(UINT sdk_ver);
static HWND (STDCALL *real_CreateWindowExA)(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam);
static HRESULT (STDCALL* real_SetRenderState)(IDirect3DDevice8* self,
        D3DRENDERSTATETYPE State, DWORD Value);
static HRESULT (STDCALL *real_DrawPrimitiveUP)(
        IDirect3DDevice8* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride);
static HRESULT (STDCALL* real_Present)(IDirect3DDevice8* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);

static const struct hook_symbol d3d8_hook_syms[] = {
    {
        .name   = "Direct3DCreate8",
        .patch  = my_Direct3DCreate8,
        .link   = (void **) &real_Direct3DCreate8,
    },
};

static const struct hook_symbol d3d8_hook_user32_syms[] = {
    {
        .name   = "CreateWindowExA",
        .patch  = my_CreateWindowExA,
        .link   = (void **) &real_CreateWindowExA,
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
    if (gfx_windowed && gfx_window_framed) {
        /* use a different style */
        dwStyle |= WS_OVERLAPPEDWINDOW;
        /* also show mouse cursor */
        ShowCursor(TRUE);
    }

    if (gfx_window_width != -1 && gfx_window_height != -1) {
        log_misc("Overriding window size from %dx%d with %dx%d", nWidth,
            nHeight, gfx_window_width, gfx_window_height);

        nWidth = gfx_window_width;
        nHeight = gfx_window_height;
    }

    HWND hwnd = real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName,
            dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance,
            lpParam);

    if (hwnd == INVALID_HANDLE_VALUE) {
        return hwnd;
    }

    if (gfx_windowed && gfx_window_framed) {
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

static HRESULT STDCALL my_CreateDevice(
    IDirect3D8 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd, DWORD flags,
    D3DPRESENT_PARAMETERS *pp, IDirect3DDevice8 **pdev)
{
    IDirect3D8 *real = COM_PROXY_UNWRAP(self);
    HRESULT hr;
    IDirect3DDevice8* api;
    IDirect3DDevice8Vtbl *api_vtbl;
    struct com_proxy *api_proxy;
    char* error;

    log_info("Direct3D8 CreateDevice hook hit");

    /* Fix a long-standing bug in IIDX */

    if (flags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) {
        flags &= ~D3DCREATE_PUREDEVICE;
    }

    if (gfx_windowed) {
        log_misc("Window mode");

        pp->hDeviceWindow = 0;
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
        pp->FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }

    /* If we don't do this, the game will crash on some versions of windows
       on either fullscreen or windowed or even both. Also, further reports
       about textures with green glowing borders are gone as well when applying
       this */
    D3DDISPLAYMODE mode;
    IDirect3D8_GetAdapterDisplayMode(real, adapter, &mode);
    pp->BackBufferFormat = mode.Format;

    hr = IDirect3D8_CreateDevice(real, adapter, type, hwnd, flags, pp, pdev);

    if (hr != S_OK) {
        switch (hr) {
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

        log_warning("Creating D3D8 device failed: %lX, %s", hr, error);

        return hr;
    }

    api = *pdev;
    api_proxy = com_proxy_wrap(api, sizeof(*api->lpVtbl));
    api_vtbl = api_proxy->vptr;

    real_DrawPrimitiveUP = api_vtbl->DrawPrimitiveUP;
    api_vtbl->DrawPrimitiveUP = my_DrawPrimitiveUP;

    real_SetRenderState = api_vtbl->SetRenderState;
    api_vtbl->SetRenderState = my_SetRenderState;

    real_Present = api_vtbl->Present;
    api_vtbl->Present = my_Present;

    *pdev = (IDirect3DDevice8*) api_proxy;

    if (gfx_monitor_check_cb) {
        execute_monitor_check(api_vtbl, *pdev);
    }

    return hr;
}

static IDirect3D8 *STDCALL my_Direct3DCreate8(UINT sdk_ver)
{
    IDirect3D8 *api;
    IDirect3D8Vtbl *api_vtbl;
    struct com_proxy *api_proxy;

    log_info("Direct3DCreate8 hook hit");

    api = real_Direct3DCreate8(sdk_ver);
    api_proxy = com_proxy_wrap(api, sizeof(*api->lpVtbl));
    api_vtbl = api_proxy->vptr;

    api_vtbl->CreateDevice = my_CreateDevice;

    return (IDirect3D8 *) api_proxy;
}

static HRESULT STDCALL my_SetRenderState(IDirect3DDevice8* self,
        D3DRENDERSTATETYPE State, DWORD Value)
{
    if (gfx_fix_iidx_12_fog) {
        if (State == D3DRS_FOGENABLE) {
            Value = FALSE;
        }
    }

    if (gfx_fix_iidx_13_lighting) {
        if (State == D3DRS_LIGHTING) {
            Value = FALSE;
        }
    }

    return real_SetRenderState(self, State, Value);
}

static HRESULT STDCALL my_DrawPrimitiveUP(
        IDirect3DDevice8* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride)
{
    /* trap the background video texture to fix the UVs for newer hardware
       background videos are rendered to a 512x512 texture, but the videos
       are 312x416 in size (on IIDX RED)
       the old hardware supported this to render correctly
       (VMs might work as well) with the UVs set in the code but on newer
       hardware, the video frame gets stretched to the full 512x512 texture
       The tex UVs are only set to a subsection of that texture thus just a
       subsection is rendered (video appears to be stretched)
       to fix this, we simply set the UVs to apply to the full texture
       Quad for BG Video (triangle fan!):
       164/0 ------- 476/0
          |                |
          |                |
          |                |
          |                |
          |                |
          |                |
       164/416 ------- 476/416
       Original UVs for the quad
       0/0,8125 ------- 0,609375/0,812500
          |                |
          |                |
          |                |
          |                |
          |                |
          |                |
       0/0 ------- 0,609375/0
    */
    if (    gfx_fix_stretched_bg_videos && primitive_type == 6 &&
            primitive_count == 2 && stride == 28) {

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

static HRESULT STDCALL my_Present(IDirect3DDevice8* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
    static uint64_t current_time = 0;

    HRESULT hr = real_Present(self, pSourceRect, pDestRect, hDestWindowOverride,
        pDirtyRegion);

    if (gfx_frame_rate_limit > 0.0f) {
        if (current_time == 0) {
            current_time = time_get_counter();
        } else {
            uint64_t frame_time = 1000000 / gfx_frame_rate_limit;

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

/* ------------------------------------------------------------------------- */

void d3d8_hook_init(void)
{
    hook_table_apply(
            NULL,
            "d3d8.dll",
            d3d8_hook_syms,
            lengthof(d3d8_hook_syms));

    hook_table_apply(
            NULL,
            "user32.dll",
            d3d8_hook_user32_syms,
            lengthof(d3d8_hook_user32_syms));

    log_info("Inserted d3d8 graphics hooks");
}

void d3d8_set_windowed(bool framed, int32_t width, int32_t height)
{
    gfx_windowed = true;
    gfx_window_framed = framed;
    gfx_window_width = width;
    gfx_window_height = height;
}

void d3d8_set_frame_rate_limit(float limit)
{
    if (limit < 0.0f) {
        limit = 0.0f;
    }

    log_info("Limiting rendering frame rate to %f frames", limit);
    gfx_frame_rate_limit = limit;
}

void d3d8_enable_monitor_check(d3d8_monitor_check_result_callback_t cb)
{
    log_assert(cb);

    gfx_monitor_check_cb = cb;
    log_info("Enabled monitor check");
}

void d3d8_iidx_fix_stretched_bg_videos(void)
{
    gfx_fix_stretched_bg_videos = true;
    log_info("Fixing UVs of stretched BG videos");
}

void d3d8_iidx_fix_12_song_select_bg(void)
{
    gfx_fix_iidx_12_fog = true;
    log_info("Fixing Happy Sky bugged music select 3D background");
}

void d3d8_iidx_fix_13_song_select_bg(void)
{
    gfx_fix_iidx_13_lighting = true;
    log_info("Fixing DistorteD music select 3D background");
}

/* ------------------------------------------------------------------------- */

static void execute_monitor_check(IDirect3DDevice8Vtbl* api_vtbl,
        IDirect3DDevice8* pdev)
{
    log_info("Running monitor check...");

    /* d3d8 does not provide anything like d3d9 with DrawText */

    const uint32_t max_iterations = 60 * 30;
    const uint32_t skip_frames = 60 * 1;

    uint64_t accu_us = 0;

    double result = 0;
    uint32_t iterations = 0;

    while (iterations < max_iterations) {
        iterations++;
        uint64_t start = time_get_counter();

        api_vtbl->Clear(pdev, 0, NULL, D3DCLEAR_TARGET,
            D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), 0.0f, 0);
        api_vtbl->BeginScene(pdev);

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

    log_assert(gfx_monitor_check_cb);

    gfx_monitor_check_cb(result);
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

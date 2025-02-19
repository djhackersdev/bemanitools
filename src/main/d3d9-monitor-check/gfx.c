#include <windows.h>

#include <d3d9.h>
#include <d3dx9core.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"
#include "d3d9-monitor-check/print-console.h"

#include "util/mem.h"
#include "util/time.h"

typedef struct gfx_ctx_t {
    HWND hwnd;
    IDirect3D9 *d3d;
    IDirect3DDevice9 *device;
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;
    bool windowed;
    bool vsync;
} gfx_ctx_t;

typedef struct gfx_render_state_t {
    uint64_t frame_current;
    uint64_t frame_start_time;
    uint64_t last_frame_time_us;
} gfx_render_state_t;

typedef struct gfx {
    gfx_ctx_t ctx;
    gfx_render_state_t render_state;
} gfx_t;

static const D3DFORMAT _gfx_d3dformat = D3DFMT_X8R8G8B8;

static bool _gfx_d3d_context_create(IDirect3D9 **d3d)
{
    // Initialize D3D
    *d3d = Direct3DCreate9(D3D_SDK_VERSION);

    if (!*d3d) {
        printfln_winerr("Creating d3d context failed");
        return false;
    }

    return true;
}

static bool _gfx_adapter_identifier_query(IDirect3D9 *d3d, D3DADAPTER_IDENTIFIER9 *identifier)
{
    HRESULT hr;

    hr = IDirect3D9_GetAdapterIdentifier(d3d, D3DADAPTER_DEFAULT, 0, identifier);

    if (hr != D3D_OK) {
        printfln_winerr("GetAdapterIdentifier failed");
        return false;
    }

    return true;
}

static bool _gfx_window_create(uint32_t width, uint32_t height, HWND *hwnd)
{
    WNDCLASSEX wc;

    memset(&wc, 0, sizeof(wc));

    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "D3D9MonitorCheck";

    RegisterClassExA(&wc);

    // Create window
    *hwnd = CreateWindowA(
        wc.lpszClassName,
        "D3D9 Monitor Check",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        NULL,
        NULL,
        wc.hInstance,
        NULL);

    if (!*hwnd) {
        printfln_winerr("Failed to create window");
        return false;
    }

    return true;
}

static bool _gfx_d3d_device_create(
    HWND hwnd,
    IDirect3D9 *d3d,
    uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    bool windowed,
    bool vsync,
    IDirect3DDevice9 **device)
{
    D3DPRESENT_PARAMETERS pp;
    HRESULT hr;

    memset(&pp, 0, sizeof(pp));

    if (windowed) {
        ShowWindow(hwnd, SW_SHOW);

        pp.Windowed = TRUE;
        pp.FullScreen_RefreshRateInHz = 0;
    } else {
        ShowCursor(FALSE);

        pp.Windowed = FALSE;
        pp.FullScreen_RefreshRateInHz = refresh_rate;
    }

    if (vsync) {
        pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    } else {
        pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }

    pp.BackBufferWidth = width;
    pp.BackBufferHeight = height;
    pp.BackBufferFormat = _gfx_d3dformat;
    pp.BackBufferCount = 2;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = hwnd;
    pp.EnableAutoDepthStencil = TRUE;
    pp.AutoDepthStencilFormat = D3DFMT_D16;
    pp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    // Create D3D device
    hr = IDirect3D9_CreateDevice(
        d3d,
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &pp,
        device);

    if (hr != D3D_OK) {
        printfln_winerr("Creating d3d device failed");
        return false;
    }

    return true;
}

bool gfx_adapter_info_get(D3DADAPTER_IDENTIFIER9 *adapter)
{
    IDirect3D9 *d3d;

    assert(adapter);

    memset(adapter, 0, sizeof(D3DADAPTER_IDENTIFIER9));

    if (!_gfx_d3d_context_create(&d3d)) {
        return false;
    }
    
    if (!_gfx_adapter_identifier_query(d3d, adapter)) {
        IDirect3D9_Release(d3d);
        return false;
    }
    
    IDirect3D9_Release(d3d);

    return true;
}

bool gfx_adapter_modes_get(gfx_adapter_modes_t *modes)
{
    IDirect3D9 *d3d;
    HRESULT hr;
    UINT mode_count;

    assert(modes);

    memset(modes, 0, sizeof(gfx_adapter_modes_t));

    if (!_gfx_d3d_context_create(&d3d)) {
        return false;
    }

    mode_count = IDirect3D9_GetAdapterModeCount(d3d, D3DADAPTER_DEFAULT, _gfx_d3dformat);

    if (mode_count > sizeof(modes->modes)) {
        mode_count = sizeof(modes->modes);
        printfln_err("WARNING: Available adapter modes (total %d) is greater than the maximum supported modes in structure (%zu)", mode_count, sizeof(modes->modes));
    }

    for (UINT i = 0; i < mode_count; i++) {
        hr = IDirect3D9_EnumAdapterModes(d3d, D3DADAPTER_DEFAULT, _gfx_d3dformat, i, &modes->modes[i]);

        if (hr != D3D_OK) {
            printfln_winerr("EnumAdapterMode index %d failed", i);
            IDirect3D9_Release(d3d);
            return false;
        }
    }

    modes->count = mode_count;

    IDirect3D9_Release(d3d);

    return true;
}

bool gfx_init(
    uint32_t width,
    uint32_t height,
    uint32_t refresh_rate,
    bool windowed,
    bool vsync,
    gfx_t **gfx)
{
    HWND hwnd;
    IDirect3D9 *d3d;
    D3DADAPTER_IDENTIFIER9 identifier;
    IDirect3DDevice9 *device;

    assert(gfx);

    printfln_err("Creating d3d context ...");

    if (!_gfx_d3d_context_create(&d3d)) {
        return false;
    }
    
    printfln_err("Querying adapter identifier ...");

    if (!_gfx_adapter_identifier_query(d3d, &identifier)) {
        IDirect3D9_Release(d3d);
        return false;
    }

    printfln_err("Adapter:");
    printfln_err("Driver: %s", identifier.Driver);
    printfln_err("Description: %s", identifier.Description);
    printfln_err("DeviceName: %s", identifier.DeviceName);
#ifdef _WIN32
    printfln_err("DriverVersion: %lld", identifier.DriverVersion.QuadPart);
#else
    printfln_err("DriverVersion: %lu.%lu", identifier.DriverVersionHighPart, identifier.DriverVersionLowPart);
#endif

    printfln_err("Creating window with %dx%d ...", width, height);

    if (!_gfx_window_create(width, height, &hwnd)) {
        IDirect3D9_Release(d3d);
        return false;
    }

    printfln_err("Creating d3d device %d x %d @ %d hz %s vsync %s ...",
        width,
        height,
        refresh_rate,
        windowed ? "windowed" : "fullscreen",
        vsync ? "on" : "off");

    if (!_gfx_d3d_device_create(
            hwnd,
            d3d,
            width,
            height,
            refresh_rate,
            windowed,
            vsync,
            &device)) {
        IDirect3D9_Release(d3d);
        DestroyWindow(hwnd);
        return false;
    }

    *gfx = xmalloc(sizeof(gfx_t));
    memset(*gfx, 0, sizeof(gfx_t));

    (*gfx)->ctx.hwnd = hwnd;
    (*gfx)->ctx.d3d = d3d;
    (*gfx)->ctx.device = device;
    (*gfx)->ctx.width = width;
    (*gfx)->ctx.height = height;
    (*gfx)->ctx.refresh_rate = refresh_rate;
    (*gfx)->ctx.windowed = windowed;
    (*gfx)->ctx.vsync = vsync;

    (*gfx)->render_state.frame_current = 0;
    (*gfx)->render_state.frame_start_time = 0;
    (*gfx)->render_state.last_frame_time_us = 0;

    return true;
}

uint32_t gfx_width_get(gfx_t *gfx)
{
    assert(gfx);

    return gfx->ctx.width;
}

uint32_t gfx_height_get(gfx_t *gfx)
{
    assert(gfx);

    return gfx->ctx.height;
}

bool gfx_info_get(gfx_t *gfx, gfx_info_t *info)
{
    D3DADAPTER_IDENTIFIER9 identifier;

    assert(gfx);
    assert(info);

    if (!_gfx_adapter_identifier_query(gfx->ctx.d3d, &identifier)) {
        return false;
    }

    strncpy(info->adapter_identifier, identifier.Description, sizeof(info->adapter_identifier));

    info->width = gfx->ctx.width;
    info->height = gfx->ctx.height;
    info->refresh_rate = gfx->ctx.refresh_rate;
    info->windowed = gfx->ctx.windowed;
    info->vsync = gfx->ctx.vsync;

    return true;
}

IDirect3DDevice9 *gfx_device_get(gfx_t *gfx)
{
    assert(gfx);

    return gfx->ctx.device;
}

bool gfx_frame_begin(gfx_t *gfx)
{
    MSG msg;

    assert(gfx);

    // Required to not make windows think we are stuck and not responding
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    IDirect3DDevice9_Clear(
        gfx->ctx.device, 
        0, 
        NULL, 
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(0, 0, 0), 
        1.0f, 
        0);     
    IDirect3DDevice9_BeginScene(gfx->ctx.device);

    return true;
}

void gfx_frame_end(gfx_t *gfx)
{
    uint64_t end_time;

    assert(gfx);

    IDirect3DDevice9_EndScene(gfx->ctx.device);
    IDirect3DDevice9_Present(gfx->ctx.device, NULL, NULL, NULL, NULL);

    // End previous frame time measurement after the frame has been presented (which might include waiting for vsync)
    // and start a new frame time measurement immediately
    // This ensures that anything that comes after ending the frame and still before starting a new frame, e.g. IO
    // processing, is included in the total frame time measurement
    // However, this can not include the very first frame
    if (gfx->render_state.frame_current > 0) {
        end_time = time_get_counter();
        gfx->render_state.last_frame_time_us = time_get_elapsed_us(end_time - gfx->render_state.frame_start_time);
        gfx->render_state.frame_start_time = end_time;
    }

    gfx->render_state.frame_start_time = time_get_counter();

    gfx->render_state.frame_current++;
}

uint64_t gfx_last_frame_count_get(gfx_t *gfx)
{
    assert(gfx);

    return gfx->render_state.frame_current;
}

uint64_t gfx_last_frame_time_us_get(gfx_t *gfx)
{
    assert(gfx);

    return gfx->render_state.last_frame_time_us;
}

void gfx_fini(gfx_t *gfx)
{
    assert(gfx);

    IDirect3DDevice9_Release(gfx->ctx.device);
    IDirect3D9_Release(gfx->ctx.d3d);
    DestroyWindow(gfx->ctx.hwnd);

    free(gfx);
}
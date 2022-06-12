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

#include "iidxhook-util/d3d9.h"
#include "iidxhook-util/vertex-shader.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

/* ------------------------------------------------------------------------- */

struct d3d9_vertex {
    float x;
    float y;
    float z;
    uint32_t color;
    uint32_t unknown;
    float tu;
    float tv;
};

/* ------------------------------------------------------------------------- */

typedef HRESULT WINAPI (*func_D3DXCreateFontA)(
    struct IDirect3DDevice9 *device,
    INT height,
    UINT width,
    UINT weight,
    UINT miplevels,
    BOOL italic,
    DWORD charset,
    DWORD precision,
    DWORD quality,
    DWORD pitchandfamily,
    const char *facename,
    struct ID3DXFont **font);

/* ------------------------------------------------------------------------- */

static struct iidxhook_util_d3d9_config iidxhook_util_d3d9_config;

static uint64_t iidxhook_util_d3d9_present_current_time = 0;

static struct {
    uint16_t original_back_buffer_width;
    uint16_t original_back_buffer_height;

    IDirect3DTexture9 *rt_texture;
    IDirect3DSurface9 *rt_surface;
    IDirect3DSurface9 *rt_orig_surface;

    D3DTEXTUREFILTERTYPE filter;
} iidxhook_util_d3d9_back_buffer_scaling;

static IDirect3DVertexShader9* vertex_shader;

/* ------------------------------------------------------------------------------------------------------------------
 */

static bool iidxhook_util_d3d9_float_equal(float a, float b, float eps)
{
    return fabs(a - b) < eps;
}

static void iidxhook_util_d3d9_calc_win_size_with_framed(
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

/* ------------------------------------------------------------------------------------------------------------------
 */

static void iidxhook_util_d3d9_patch_gpu_vid_pid(struct hook_d3d9_irp *irp)
{
    char pci_id[32];

    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES);

    if (iidxhook_util_d3d9_config.pci_vid > 0 &&
        iidxhook_util_d3d9_config.pci_pid > 0) {

        str_format(
            pci_id,
            sizeof(pci_id),
            "PCI\\VEN_%04X&DEV_%04X",
            iidxhook_util_d3d9_config.pci_vid,
            iidxhook_util_d3d9_config.pci_pid);

        /* Apparently Konami didn't read the "Not Used" message in the MSDN
        docs for DISPLAY_DEVICE */
        log_misc(
            "Replacing device ID %s with %s",
            irp->args.enum_display_devices.info->DeviceID,
            pci_id);

        str_cpy(
            irp->args.enum_display_devices.info->DeviceID,
            sizeof(irp->args.enum_display_devices.info->DeviceID),
            pci_id);
    }
}

static void iidxhook_util_d3d9_patch_window(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX);

    if (iidxhook_util_d3d9_config.windowed &&
        iidxhook_util_d3d9_config.framed) {
        /* use a different style */
        irp->args.create_window_ex.style |= WS_OVERLAPPEDWINDOW;

        /* also show mouse cursor */
        ShowCursor(TRUE);
    }

    if (iidxhook_util_d3d9_config.override_window_width > 0 &&
        iidxhook_util_d3d9_config.override_window_height) {
        log_misc(
            "Overriding window size from %dx%d with %dx%d",
            irp->args.create_window_ex.width,
            irp->args.create_window_ex.height,
            iidxhook_util_d3d9_config.override_window_width,
            iidxhook_util_d3d9_config.override_window_height);

        irp->args.create_window_ex.width =
            iidxhook_util_d3d9_config.override_window_width;
        irp->args.create_window_ex.height =
            iidxhook_util_d3d9_config.override_window_height;
    }
}

static void
iidxhook_util_d3d9_fix_window_size_and_pos(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX);

    if (iidxhook_util_d3d9_config.windowed &&
        iidxhook_util_d3d9_config.framed) {
        /* we have to adjust the window size, because the window needs to be a
           slightly bigger than the rendering resolution (window caption and
           stuff is included in the window size) */
        WINDOWPOS wp;

        iidxhook_util_d3d9_calc_win_size_with_framed(
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

static void
iidxhook_util_d3d9_log_create_device_params(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);

    log_misc(
        "CreateDevice parameters: adapter %d, type %d, hwnd %p, flags %lX, "
        "pdev %p",
        irp->args.ctx_create_device.adapter,
        irp->args.ctx_create_device.type,
        irp->args.ctx_create_device.hwnd,
        irp->args.ctx_create_device.flags,
        irp->args.ctx_create_device.pdev);

    log_misc(
        "D3D9 presenter parameters: BackBufferWidth %d, BackBufferHeight "
        "%d, BackBufferFormat %d, "
        "BackBufferCount %d, MultiSampleType %d, SwapEffect %d, "
        "hDeviceWindow %p, Windowed %d, "
        "EnableAutoDepthStencil "
        "%d, AutoDepthStencilFormat %d, Flags %lX, "
        "FullScreen_RefreshRateInHz %d",
        irp->args.ctx_create_device.pp->BackBufferWidth,
        irp->args.ctx_create_device.pp->BackBufferHeight,
        irp->args.ctx_create_device.pp->BackBufferFormat,
        irp->args.ctx_create_device.pp->BackBufferCount,
        irp->args.ctx_create_device.pp->MultiSampleType,
        irp->args.ctx_create_device.pp->SwapEffect,
        irp->args.ctx_create_device.pp->hDeviceWindow,
        irp->args.ctx_create_device.pp->Windowed,
        irp->args.ctx_create_device.pp->EnableAutoDepthStencil,
        irp->args.ctx_create_device.pp->AutoDepthStencilFormat,
        irp->args.ctx_create_device.pp->Flags,
        irp->args.ctx_create_device.pp->FullScreen_RefreshRateInHz);
}

static void
iidxhook_util_d3d9_fix_iidx_bug_software_processing(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);

    /* Fix a long-standing bug in IIDX */
    if (irp->args.ctx_create_device.flags &
        D3DCREATE_SOFTWARE_VERTEXPROCESSING) {
        irp->args.ctx_create_device.flags &= ~D3DCREATE_PUREDEVICE;
    }
}

static void iidxhook_util_d3d9_fix_create_device_apply_window_mode(
    struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);
    D3DPRESENT_PARAMETERS *pp = irp->args.ctx_create_device.pp;

    if (iidxhook_util_d3d9_config.windowed) {
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    } else {
        if (iidxhook_util_d3d9_config.forced_refresh_rate > 0) {
            log_info(
                "Forcing refresh rate %d -> %d",
                pp->FullScreen_RefreshRateInHz,
                iidxhook_util_d3d9_config.forced_refresh_rate);
            pp->FullScreen_RefreshRateInHz =
                iidxhook_util_d3d9_config.forced_refresh_rate;
        }
    }

    if (iidxhook_util_d3d9_config.device_adapter >= 0) {
        log_info(
            "Forcing adapter %d -> %d",
            irp->args.ctx_create_device.adapter,
            iidxhook_util_d3d9_config.device_adapter);
        irp->args.ctx_create_device.adapter =
            iidxhook_util_d3d9_config.device_adapter;
    }
}

static void iidxhook_util_d3d9_fix_back_buffer_format(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);

    /* Same fix as on D3D8, orz...
       If we don't do this, some games one certain platforms (e.g. iidx 14/15 on
       Windows 10). CreateDevice fails with an "invalid call" on either
       fullscreen or windowed or even both.
       Also, further reports about textures with green glowing borders are gone
       as well when applying this */
    D3DDISPLAYMODE mode;
    IDirect3D9_GetAdapterDisplayMode(
        irp->args.ctx_create_device.self,
        irp->args.ctx_create_device.adapter,
        &mode);

    irp->args.ctx_create_device.pp->BackBufferFormat = mode.Format;
}

static ID3DXFont *iidxhook_util_d3d9_load_and_create_font(IDirect3DDevice9 *dev)
{
    HMODULE d3d9_24;
    HRESULT hr;
    ID3DXFont *font;

    d3d9_24 = GetModuleHandleA("d3dx9_24.dll");

    if (d3d9_24 == NULL) {
        log_warning(
            "Failed to load d3dx9_24.dll to create a font for displaying "
            "framerate on monitor check.");
        return NULL;
    }

    func_D3DXCreateFontA d3dxCreateFontA =
        (func_D3DXCreateFontA) GetProcAddress(d3d9_24, "D3DXCreateFontA");

    if (d3dxCreateFontA == NULL) {
        log_warning("Failed to find function D3DXCreateFontA");
        return NULL;
    }

    hr = d3dxCreateFontA(
        dev,
        22,
        0,
        FW_NORMAL,
        1,
        false,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE,
        "Arial",
        &font);

    if (hr != S_OK) {
        log_warning("Failed to load font for monitor check: %lX", hr);
        return NULL;
    }

    return font;
}

static void iidxhook_util_d3d9_execute_monitor_check(struct hook_d3d9_irp *irp)
{
    const uint32_t max_iterations = 60 * 30;
    const uint32_t skip_frames = 60 * 1;

    uint64_t accu_us;
    double result;
    uint32_t iterations;
    uint64_t start;

    char text_buffer[256];
    ID3DXFont *font;
    RECT font_rect;

    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);

    if (iidxhook_util_d3d9_config.iidx09_to_19_monitor_check_cb) {
        log_info("Running monitor check...2");

        accu_us = 0;

        result = 0;
        iterations = 0;

        font = iidxhook_util_d3d9_load_and_create_font(
            *irp->args.ctx_create_device.pdev);

        if (font == NULL) {
            log_info(
                "Monitor check without on-screen text showing current "
                "refresh rate.");
        } else {
            SetRect(&font_rect, 20, 20, 640, 480);
        }

        while (iterations < max_iterations) {
            if (font) {
                sprintf(
                    text_buffer,
                    "Monitor check...\n"
                    "Elapsed iterations: %d/%d\n"
                    "Refresh rate: %f",
                    iterations + 1,
                    max_iterations,
                    result);
            }

            iterations++;
            start = time_get_counter();

            IDirect3DDevice9_BeginScene(*irp->args.ctx_create_device.pdev);

            IDirect3DDevice9_Clear(
                *irp->args.ctx_create_device.pdev,
                0,
                NULL,
                D3DCLEAR_TARGET,
                D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF),
                0.0f,
                0);

            if (font) {
                ID3DXFont_DrawTextA(
                    font,
                    NULL,
                    text_buffer,
                    -1,
                    &font_rect,
                    DT_LEFT,
                    D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF));
            }

            IDirect3DDevice9_EndScene(*irp->args.ctx_create_device.pdev);

            IDirect3DDevice9_Present(
                *irp->args.ctx_create_device.pdev, NULL, NULL, NULL, NULL);

            /* Skip the first inaccurate values */
            if (iterations > skip_frames) {
                accu_us += time_get_elapsed_us(time_get_counter() - start);

                result = ((double) (iterations - skip_frames)) /
                    (accu_us / 1000.0 / 1000.0);
            }
        }

        log_info(
            "Monitor check done (total iterations %d), refesh rate: %f hz",
            iterations,
            result);

        /* Sanity check to ensure people notice that their current refresh rate
           is way off. */
        if (result < 55 || result > 65) {
            log_warning(
                "Monitor check result (%f hz) is not even near the "
                "intended refresh rate of 60 hz. Fix your setup to ensure "
                "a "
                "constant and as close to as possible 60 hz refresh rate.",
                result);
        }

        /* Leave results of monitor check on screen for a moment */
        Sleep(2000);

        log_assert(iidxhook_util_d3d9_config.iidx09_to_19_monitor_check_cb);

        iidxhook_util_d3d9_config.iidx09_to_19_monitor_check_cb(result);
    }
}

static void iidxhook_util_d3d9_log_result_create_device(HRESULT hr)
{
    char *error;

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
    }
}

static void
iidxhook_util_d3d9_nvidia_fix_iidx14_to_19(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE);

    /* Fix non ATI cards not working on Gold (and DJT?). There is a check that
        creates textures when starting the game. This check fails which results
        in a message box being shown mentioning something about an aep-lib
       error. Seems like a combination of parameters is not supported by non ATI
       cards and throws an error (Fix taken from old Gold cracks by ahnada and
       tau)
    */

    if (iidxhook_util_d3d9_config.iidx14_to_19_nvidia_fix &&
        irp->args.dev_create_texture.width == 256 &&
        irp->args.dev_create_texture.height == 256 &&
        irp->args.dev_create_texture.levels == 1 &&
        irp->args.dev_create_texture.usage == D3DUSAGE_RENDERTARGET &&
        irp->args.dev_create_texture.format == D3DFMT_A1R5G5B5 &&
        irp->args.dev_create_texture.pool == D3DPOOL_DEFAULT &&
        irp->args.dev_create_texture.shared_handle == NULL) {
        irp->args.dev_create_texture.format = D3DFMT_A8R8G8B8;
    }
}

static void iidxhook_util_d3d9_framerate_limiter(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_PRESENT);

    if (iidxhook_util_d3d9_config.framerate_limit > 0.0f) {
        if (iidxhook_util_d3d9_present_current_time == 0) {
            iidxhook_util_d3d9_present_current_time = time_get_counter();
        } else {
            uint64_t frame_time =
                1000000 / iidxhook_util_d3d9_config.framerate_limit;

            uint64_t dt = time_get_elapsed_us(
                time_get_counter() - iidxhook_util_d3d9_present_current_time);

            while (dt < frame_time) {
                /* waste some cpu time by polling
                   because we can't sleep for X us */
                dt = time_get_elapsed_us(
                    time_get_counter() -
                    iidxhook_util_d3d9_present_current_time);
            }

            iidxhook_util_d3d9_present_current_time = time_get_counter();
        }
    }
}

static void
iidxhook_util_d3d9_iidx12_fix_song_select_bg(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE);

    if (iidxhook_util_d3d9_config.iidx12_fix_song_select_bg) {
        if (irp->args.dev_set_render_state.state == D3DRS_FOGENABLE) {
            irp->args.dev_set_render_state.value = FALSE;
        }
    }
}

static void
iidxhook_util_d3d9_iidx13_fix_song_select_bg(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE);

    if (iidxhook_util_d3d9_config.iidx13_fix_song_select_bg) {
        if (irp->args.dev_set_render_state.state == D3DRS_LIGHTING) {
            irp->args.dev_set_render_state.value = FALSE;
        }
    }
}

static void
iidxhook_util_d3d9_iidx11_to_17_fix_uvs_bg_videos(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_DRAW_PRIMITIVE_UP);

    /* same code taken from the d3d8 module. but, this just fixes the quad
       seam issue as there are no reports of streched bg videos */
    if (iidxhook_util_d3d9_config.iidx11_to_17_fix_uvs_bg_videos &&
        irp->args.dev_draw_primitive_up.primitive_type == D3DPT_TRIANGLEFAN &&
        irp->args.dev_draw_primitive_up.primitive_count == 2 &&
        irp->args.dev_draw_primitive_up.stride == 28) {

        struct d3d9_vertex *vertices =
            (struct d3d9_vertex *) irp->args.dev_draw_primitive_up.data;

        /*
        log_info("Video Tex: %f/%f %f/%f %f/%f %f/%f",
            vertices[0].x, vertices[0].y,
            vertices[1].x, vertices[1].y,
            vertices[2].x, vertices[2].y,
            vertices[3].x, vertices[3].y);
        */

        /* Fix full screen background videos (e.g. DistorteD intro sequence) */
        if (vertices[0].x >= 0.0f && vertices[0].x < 1.0f &&
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

            /* another identifier, because there are other textures with
               512x512 size make sure we got the bg video only to not mess
               up anything else */
            /* different versions have different themes and position the bg
               video on slightly different positions (good job...) */
            if (/* single */
                ((vertices[0].x >= 164.0f && vertices[0].x <= 168.0f &&
                  iidxhook_util_d3d9_float_equal(vertices[0].y, 0.0f, 0.1f)) &&
                 (vertices[1].x >= 472.0f && vertices[1].x <= 476.0f &&
                  iidxhook_util_d3d9_float_equal(vertices[1].y, 0.0f, 0.1f)) &&
                 (vertices[2].x >= 472.0f && vertices[2].x <= 476.0f &&
                  iidxhook_util_d3d9_float_equal(
                      vertices[2].y, 416.0f, 0.1f)) &&
                 (vertices[3].x >= 164.0f && vertices[3].x <= 168.0f &&
                  iidxhook_util_d3d9_float_equal(
                      vertices[3].y, 416.0f, 0.1f))) ||
                /* double top left */
                ((iidxhook_util_d3d9_float_equal(vertices[0].x, 6.0f, 0.1f) &&
                  vertices[0].y >= 24.0f && vertices[0].y <= 28.0f) &&
                 (iidxhook_util_d3d9_float_equal(vertices[1].x, 147.0f, 0.1f) &&
                  vertices[1].y >= 24.0f && vertices[1].y <= 28.0f) &&
                 (iidxhook_util_d3d9_float_equal(vertices[2].x, 147.0f, 0.1f) &&
                  vertices[2].y >= 212.0f && vertices[2].y <= 216.0f) &&
                 (iidxhook_util_d3d9_float_equal(vertices[3].x, 6.0f, 0.1f) &&
                  vertices[3].y >= 212.0f && vertices[3].y <= 216.0f)) ||
                /* double bottom left */
                ((iidxhook_util_d3d9_float_equal(vertices[0].x, 6.0f, 0.1f) &&
                  vertices[0].y >= 216.0f && vertices[0].y <= 220.0f) &&
                 (iidxhook_util_d3d9_float_equal(vertices[1].x, 147.0f, 0.1f) &&
                  vertices[1].y >= 216.0f && vertices[1].y <= 220.0f) &&
                 (iidxhook_util_d3d9_float_equal(vertices[2].x, 147.0f, 0.1) &&
                  vertices[2].y >= 404.0f && vertices[2].y <= 408.0f) &&
                 (iidxhook_util_d3d9_float_equal(vertices[3].x, 6.0f, 0.1f) &&
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
                  vertices[1].y >= 216.0f && vertices[1].y <= 220.0f) &&
                 (vertices[2].x >= 634.0f && vertices[2].x <= 635.0f &&
                  vertices[2].y >= 404.0f && vertices[2].y <= 408.0f) &&
                 (vertices[3].x >= 493.0f && vertices[3].x <= 494.0f &&
                  vertices[3].y >= 404.0f && vertices[3].y <= 408.0f))) {
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
}

static void
iidxhook_uti_d3d9_fix_iidx9_to_13_viewport_size(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_GET_CLIENT_RECT);

    /* IIDX 9-13 (at least) use this call to get the size of rectangle for
     * setting the viewport size... */

    /* Always use the original requested back buffer size. If scaling is active,
       using the scaled
       values leads to (3D) scenes getting render to a viewport with incorrect
       sized. */
    irp->args.get_client_rect.rect->left = 0;
    irp->args.get_client_rect.rect->top = 0;
    irp->args.get_client_rect.rect->right =
        iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_width;
    irp->args.get_client_rect.rect->bottom =
        iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_height;
}

static void
iidxhook_util_d3d9_setup_back_buffer_scaling_pre(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);

    /* Store these so we can apply scaling further down */
    iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_width =
        irp->args.ctx_create_device.pp->BackBufferWidth;
    iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_height =
        irp->args.ctx_create_device.pp->BackBufferHeight;

    switch (iidxhook_util_d3d9_config.scale_back_buffer_filter) {
        case IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE:
            iidxhook_util_d3d9_back_buffer_scaling.filter = D3DTEXF_NONE;
            break;

        case IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR:
            iidxhook_util_d3d9_back_buffer_scaling.filter = D3DTEXF_LINEAR;
            break;

        case IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_POINT:
            iidxhook_util_d3d9_back_buffer_scaling.filter = D3DTEXF_POINT;
            break;

        default:
            log_fatal(
                "Unhandled case, illegal state: %d",
                iidxhook_util_d3d9_config.scale_back_buffer_filter);
            break;
    }

    if (iidxhook_util_d3d9_config.scale_back_buffer_width > 0 &&
        iidxhook_util_d3d9_config.scale_back_buffer_height > 0) {

        log_misc(
            "Scale back buffer from %dx%d -> %dx%d",
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_width,
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_height,
            iidxhook_util_d3d9_config.scale_back_buffer_width,
            iidxhook_util_d3d9_config.scale_back_buffer_height);

        irp->args.ctx_create_device.pp->BackBufferWidth =
            iidxhook_util_d3d9_config.scale_back_buffer_width;
        irp->args.ctx_create_device.pp->BackBufferHeight =
            iidxhook_util_d3d9_config.scale_back_buffer_height;
    }
}

static void
iidxhook_util_d3d9_setup_back_buffer_scaling_post(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);

    if (iidxhook_util_d3d9_config.scale_back_buffer_width > 0 &&
        iidxhook_util_d3d9_config.scale_back_buffer_height > 0) {

        log_misc(
            "Creating intermediate render target texture: %dx%d",
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_width,
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_height);

        /* This is our new render target. Let the game render to this in its
         * native res instead of the back buffer */
        hr = IDirect3DDevice9_CreateTexture(
            *irp->args.ctx_create_device.pdev,
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_width,
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_height,
            0,
            D3DUSAGE_RENDERTARGET,
            irp->args.ctx_create_device.pp->BackBufferFormat,
            D3DPOOL_DEFAULT,
            &iidxhook_util_d3d9_back_buffer_scaling.rt_texture,
            NULL);

        if (hr != S_OK) {
            log_fatal("Creating render target texture failed: %lX", hr);
        }

        hr = IDirect3DTexture9_GetSurfaceLevel(
            iidxhook_util_d3d9_back_buffer_scaling.rt_texture,
            0,
            &iidxhook_util_d3d9_back_buffer_scaling.rt_surface);

        if (hr != S_OK) {
            log_fatal(
                "Getting surface of render target texture failed: %lX", hr);
        }

        /* Save surface of original render target */
        hr = IDirect3DDevice9_GetRenderTarget(
            *irp->args.ctx_create_device.pdev,
            0,
            &iidxhook_util_d3d9_back_buffer_scaling.rt_orig_surface);

        if (hr != S_OK) {
            log_fatal("Getting original render target failed: %lX", hr);
        }

        /* Avoid returning our scaled values because the application might use
        them, e.g. calculate sprite positions. */
        irp->args.ctx_create_device.pp->BackBufferWidth =
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_width;
        irp->args.ctx_create_device.pp->BackBufferHeight =
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_height;
    }
}

static void iidxhook_util_d3d9_set_intermediate_rt(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE);

    if (iidxhook_util_d3d9_config.scale_back_buffer_width > 0 &&
        iidxhook_util_d3d9_config.scale_back_buffer_height > 0) {
        /* Set our intermediate render texture as render target */
        hr = IDirect3DDevice9_SetRenderTarget(
            irp->args.dev_begin_scene.self,
            0,
            iidxhook_util_d3d9_back_buffer_scaling.rt_surface);

        if (hr != S_OK) {
            log_fatal("Setting intermediate render target failed: %lX", hr);
        }
    }
}

static void iidxhook_util_d3d9_set_back_buffer_rt(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_PRESENT);

    if (iidxhook_util_d3d9_config.scale_back_buffer_width > 0 &&
        iidxhook_util_d3d9_config.scale_back_buffer_height > 0) {
        /* Set our original surface/back buffer as rt */
        hr = IDirect3DDevice9_SetRenderTarget(
            irp->args.dev_present.self,
            0,
            iidxhook_util_d3d9_back_buffer_scaling.rt_orig_surface);

        if (hr != S_OK) {
            log_fatal("Setting original render target failed: %lX", hr);
        }
    }
}

static void
iidxhook_util_d3d9_scale_render_target_to_back_buffer(struct hook_d3d9_irp *irp)
{
    HRESULT hr;
    RECT src_rect;

    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_PRESENT);

    if (iidxhook_util_d3d9_config.scale_back_buffer_width > 0 &&
        iidxhook_util_d3d9_config.scale_back_buffer_height > 0) {

        src_rect.left = 0;
        src_rect.top = 0;
        src_rect.right =
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_width;
        src_rect.bottom =
            iidxhook_util_d3d9_back_buffer_scaling.original_back_buffer_height;

        hr = IDirect3DDevice9_StretchRect(
            irp->args.dev_present.self,
            iidxhook_util_d3d9_back_buffer_scaling.rt_surface,
            &src_rect,
            iidxhook_util_d3d9_back_buffer_scaling.rt_orig_surface,
            NULL,
            iidxhook_util_d3d9_back_buffer_scaling.filter);

        if (hr != S_OK) {
            log_fatal(
                "Scaling intermediate render target to back buffer failed: "
                "%lX",
                hr);
        }
    }
}

static void
iidxhook_util_d3d9_iidx18_and_19_fix_diagonal_tearing(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_SET_VIEWPORT || irp->op == HOOK_D3D9_IRP_OP_DEV_SET_VERTEX_SHADER);

    if (!iidxhook_util_d3d9_config.iidx18_and_19_diagonal_tearing_fix)
    {
        return;
    }

    if (irp->op == HOOK_D3D9_IRP_OP_DEV_SET_VIEWPORT)
    {
        const D3DVIEWPORT9 *pViewport = irp->args.dev_set_viewport.pViewport;
        const float fix_offset[2] = {-1.0F / (float)pViewport->Width, -1.0F / (float)pViewport->Height};

        hr = IDirect3DDevice9_SetVertexShaderConstantF(irp->args.dev_set_viewport.self, 63, fix_offset, lengthof(fix_offset));
        if (hr != S_OK) {
            log_warning("SetVertexShaderConstantF failed: %lX", hr);
        }
    }

    if (irp->op == HOOK_D3D9_IRP_OP_DEV_SET_VERTEX_SHADER)
    {
        if (!vertex_shader) {
            hr = IDirect3DDevice9_CreateVertexShader(irp->args.dev_set_vertex_shader.self, (const DWORD*) g_vs11_vs_main, &vertex_shader);
            if (hr != S_OK) {
                log_fatal("CreateVertexShader failed: %lX", hr);
            }
        }

        if (irp->args.dev_set_vertex_shader.pShader != NULL) {
            irp->args.dev_set_vertex_shader.pShader = vertex_shader;
        }
    }
}

/* ------------------------------------------------------------------------------------------------------------------
 */

static void
iidxhook_util_d3d9_log_config(const struct iidxhook_util_d3d9_config *config)
{
    log_misc(
        "iidxhook_util_d3d9_config\n"
        "windowed: %d\n"
        "framed: %d\n"
        "override_window_width: %d\n"
        "override_window_height: %d\n"
        "framerate_limit: %f\n"
        "pci_vid: %X\n"
        "pci_pid: %X\n"
        "iidx09_to_19_monitor_check_cb: %p\n"
        "iidx11_to_17_fix_uvs_bg_videos: %d\n"
        "iidx12_fix_song_select_bg: %d\n"
        "iidx13_fix_song_select_bg: %d\n"
        "iidx14_to_19_nvidia_fix: %d\n"
        "iidx18_and_19_diagonal_tearing_fix: %d\n"
        "scale_back_buffer_width: %d\n"
        "scale_back_buffer_height: %d\n"
        "scale_back_buffer_filter: %d\n"
        "forced_refresh_rate: %d\n"
        "device_adapter: %d",
        config->windowed,
        config->framed,
        config->override_window_width,
        config->override_window_height,
        config->framerate_limit,
        config->pci_vid,
        config->pci_pid,
        config->iidx09_to_19_monitor_check_cb,
        config->iidx11_to_17_fix_uvs_bg_videos,
        config->iidx12_fix_song_select_bg,
        config->iidx13_fix_song_select_bg,
        config->iidx14_to_19_nvidia_fix,
        config->iidx18_and_19_diagonal_tearing_fix,
        config->scale_back_buffer_width,
        config->scale_back_buffer_height,
        config->scale_back_buffer_filter,
        config->forced_refresh_rate,
        config->device_adapter);
}

static void iidxhook_util_d3d9_validate_config(
    const struct iidxhook_util_d3d9_config *config)
{
    if (!config->windowed && config->framed) {
        log_warning("Option framed does not have an effect without windowed");
    }

    if (config->override_window_width == -1) {
        log_fatal(
            "Invalid value for override_window_width: %d",
            config->override_window_width);
    }

    if (config->override_window_height == -1) {
        log_fatal(
            "Invalid value for override_window_height: %d",
            config->override_window_height);
    }

    if (!config->windowed &&
        (config->override_window_width > 0 ||
         config->override_window_height > 0)) {
        log_warning(
            "Overriding window size does not have an effect without "
            "windowed");
    }

    if (config->framerate_limit < 0.0f) {
        log_fatal(
            "Invalid value for framerate_limit: %f", config->framerate_limit);
    }
}

/* ------------------------------------------------------------------------------------------------------------------
 */

void iidxhook_util_d3d9_hook_init(void)
{
    iidxhook_util_d3d9_init_config(&iidxhook_util_d3d9_config);

    log_info("Initialized");
}

void iidxhook_util_d3d9_init_config(struct iidxhook_util_d3d9_config *config)
{
    config->windowed = false;
    config->framed = false;
    config->override_window_width = 0;
    config->override_window_height = 0;
    config->framerate_limit = 0.0f;
    config->pci_vid = 0;
    config->pci_pid = 0;
    config->iidx09_to_19_monitor_check_cb = NULL;
    config->iidx11_to_17_fix_uvs_bg_videos = false;
    config->iidx12_fix_song_select_bg = false;
    config->iidx13_fix_song_select_bg = false;
    config->iidx14_to_19_nvidia_fix = false;
    config->iidx18_and_19_diagonal_tearing_fix = false;
    config->scale_back_buffer_width = 0;
    config->scale_back_buffer_height = 0;
    config->scale_back_buffer_filter =
        IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE;
    config->forced_refresh_rate = -1;
    config->device_adapter = -1;
}

void iidxhook_util_d3d9_configure(
    const struct iidxhook_util_d3d9_config *config)
{
    log_assert(config);

    iidxhook_util_d3d9_log_config(config);
    iidxhook_util_d3d9_validate_config(config);

    memcpy(
        &iidxhook_util_d3d9_config,
        config,
        sizeof(struct iidxhook_util_d3d9_config));
}

HRESULT
iidxhook_util_d3d9_irp_handler(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);

    switch (irp->op) {
        case HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES:
            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                iidxhook_util_d3d9_patch_gpu_vid_pid(irp);
            }

            return hr;

        case HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX:
            iidxhook_util_d3d9_patch_window(irp);

            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                iidxhook_util_d3d9_fix_window_size_and_pos(irp);
            }

            return hr;

        case HOOK_D3D9_IRP_OP_GET_CLIENT_RECT:
            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                iidxhook_uti_d3d9_fix_iidx9_to_13_viewport_size(irp);
            }

            return hr;

        case HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE:
            iidxhook_util_d3d9_log_create_device_params(irp);
            iidxhook_util_d3d9_fix_iidx_bug_software_processing(irp);
            iidxhook_util_d3d9_fix_create_device_apply_window_mode(irp);
            iidxhook_util_d3d9_fix_back_buffer_format(irp);
            iidxhook_util_d3d9_setup_back_buffer_scaling_pre(irp);

            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                iidxhook_util_d3d9_setup_back_buffer_scaling_post(irp);
                iidxhook_util_d3d9_execute_monitor_check(irp);
            } else {
                iidxhook_util_d3d9_log_result_create_device(hr);
            }

            return hr;

        case HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE:
            iidxhook_util_d3d9_nvidia_fix_iidx14_to_19(irp);

            return hook_d3d9_irp_invoke_next(irp);

        case HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE:
            iidxhook_util_d3d9_set_intermediate_rt(irp);

            return hook_d3d9_irp_invoke_next(irp);

        case HOOK_D3D9_IRP_OP_DEV_PRESENT:
            iidxhook_util_d3d9_scale_render_target_to_back_buffer(irp);
            iidxhook_util_d3d9_set_back_buffer_rt(irp);

            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                iidxhook_util_d3d9_framerate_limiter(irp);
            }

            return hr;

        case HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE:
            iidxhook_util_d3d9_iidx12_fix_song_select_bg(irp);
            iidxhook_util_d3d9_iidx13_fix_song_select_bg(irp);

            return hook_d3d9_irp_invoke_next(irp);

        case HOOK_D3D9_IRP_OP_DEV_DRAW_PRIMITIVE_UP:
            iidxhook_util_d3d9_iidx11_to_17_fix_uvs_bg_videos(irp);

            return hook_d3d9_irp_invoke_next(irp);

        case HOOK_D3D9_IRP_OP_DEV_SET_VIEWPORT:
        case HOOK_D3D9_IRP_OP_DEV_SET_VERTEX_SHADER:
            iidxhook_util_d3d9_iidx18_and_19_fix_diagonal_tearing(irp);

            return hook_d3d9_irp_invoke_next(irp);

        default:
            return hook_d3d9_irp_invoke_next(irp);
    }

    log_fatal("Illegal state");
}
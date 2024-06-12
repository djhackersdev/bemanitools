#define LOG_MODULE "d3d9-hook"

#include <d3d9.h>
#include <windows.h>

#include <stdbool.h>

#include "hook/com-proxy.h"
#include "hook/pe.h"
#include "hook/table.h"

#include "iface-core/log.h"

#include "popnhook1/d3d9.h"

#include "util/defs.h"

#include "hook/d3d9.h"

static struct popnhook1_d3d9_config popnhook1_d3d9_config;

HRESULT popnhook1_d3d9_irp_handler(struct hook_d3d9_irp *irp);

static const hook_d3d9_irp_handler_t popnhook1_d3d9_handlers[] = {
    popnhook1_d3d9_irp_handler,
};

static void
popnhook1_d3d9_validate_config(const struct popnhook1_d3d9_config *config)
{
    if (!config->windowed && config->framed) {
        log_warning("Option framed does not have an effect without windowed");
    }
}

static void
popnhook1_d3d9_log_config(const struct popnhook1_d3d9_config *config)
{
    log_misc(
        "popnhook1_d3d9_config\n"
        "windowed: %d\n"
        "framed: %d\n"
        "override_window_width: %d\n"
        "override_window_height: %d\n"
        "texture_usage_fix: %d",
        config->windowed,
        config->framed,
        config->override_window_width,
        config->override_window_height,
        config->texture_usage_fix);
}

void popnhook1_d3d9_init_config(struct popnhook1_d3d9_config *config)
{
    config->windowed = false;
    config->framed = false;
    config->override_window_width = 0;
    config->override_window_height = 0;
    config->texture_usage_fix = false;
}

void popnhook1_d3d9_configure(const struct popnhook1_d3d9_config *config)
{
    log_assert(config);

    popnhook1_d3d9_log_config(config);
    popnhook1_d3d9_validate_config(config);

    memcpy(
        &popnhook1_d3d9_config, config, sizeof(struct popnhook1_d3d9_config));
}

static void popnhook1_d3d9_fix_texture_usage(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE);

    // TODO: Illegal to set render target 0 to NULL???
    irp->args.dev_create_texture.usage = 0;
}

static void popnhook1_d3d9_patch_window(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX);

    if (popnhook1_d3d9_config.windowed && popnhook1_d3d9_config.framed) {
        /* use a different style */
        irp->args.create_window_ex.style |= WS_OVERLAPPEDWINDOW;

        /* also show mouse cursor */
        ShowCursor(TRUE);
    }

    if (popnhook1_d3d9_config.override_window_width > 0 &&
        popnhook1_d3d9_config.override_window_height) {
        log_misc(
            "Overriding window size from %dx%d with %dx%d",
            irp->args.create_window_ex.width,
            irp->args.create_window_ex.height,
            popnhook1_d3d9_config.override_window_width,
            popnhook1_d3d9_config.override_window_height);

        irp->args.create_window_ex.width =
            popnhook1_d3d9_config.override_window_width;
        irp->args.create_window_ex.height =
            popnhook1_d3d9_config.override_window_height;
    }
}

static void popnhook1_d3d9_calc_win_size_with_framed(
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

static void popnhook1_d3d9_fix_window_size_and_pos(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX);

    if (popnhook1_d3d9_config.windowed && popnhook1_d3d9_config.framed) {
        /* we have to adjust the window size, because the window needs to be a
           slightly bigger than the rendering resolution (window caption and
           stuff is included in the window size) */
        WINDOWPOS wp;

        popnhook1_d3d9_calc_win_size_with_framed(
            irp->args.create_window_ex.result,
            irp->args.create_window_ex.x == CW_USEDEFAULT ?
                0 :
                irp->args.create_window_ex.x,
            irp->args.create_window_ex.y == CW_USEDEFAULT ?
                0 :
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

        ShowCursor(TRUE);
    }
}

static void
popnhook1_d3d9_fix_create_device_apply_window_mode(struct hook_d3d9_irp *irp)
{
    log_assert(irp);
    log_assert(irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE);
    D3DPRESENT_PARAMETERS *pp = irp->args.ctx_create_device.pp;

    if (popnhook1_d3d9_config.windowed) {
        pp->Windowed = TRUE;
        pp->FullScreen_RefreshRateInHz = 0;
    }
}

HRESULT
popnhook1_d3d9_irp_handler(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);

    switch (irp->op) {
        case HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX:
            popnhook1_d3d9_patch_window(irp);

            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                popnhook1_d3d9_fix_window_size_and_pos(irp);
            }

            return hr;

        case HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE:
            if (popnhook1_d3d9_config.texture_usage_fix)
                popnhook1_d3d9_fix_texture_usage(irp);

            return hook_d3d9_irp_invoke_next(irp);

        case HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE:
            popnhook1_d3d9_fix_create_device_apply_window_mode(irp);

            return hook_d3d9_irp_invoke_next(irp);

        default:
            return hook_d3d9_irp_invoke_next(irp);
    }

    log_fatal("Illegal state");
}

void popnhook1_d3d9_init(void)
{
    popnhook1_d3d9_init_config(&popnhook1_d3d9_config);

    hook_d3d9_init(popnhook1_d3d9_handlers, lengthof(popnhook1_d3d9_handlers));
}

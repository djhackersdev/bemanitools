#define LOG_MODULE "iidxhook-d3d9-bb-scale-hd"

#include <windows.h>

#include "d3d9-util/vertex.h"

#include "iface-core/log.h"

#include "bb-scale-hd.h"
#include "util.h"

static bool iidxhook_d3d9_bb_scale_initialized;
static uint16_t iidxhook_d3d9_bb_scale_hd_width;
static uint16_t iidxhook_d3d9_bb_scale_hd_height;

void iidxhook_d3d9_bb_scale_hd_init(uint16_t width, uint16_t height)
{
    iidxhook_d3d9_bb_scale_hd_width = width;
    iidxhook_d3d9_bb_scale_hd_height = height;

    iidxhook_d3d9_bb_scale_initialized = true;

    log_info("Initialized, width %d, height %d", width, height);
}

static bool iidxhook_d3d9_bb_scale_hd_in_range_of_2d_coords(
    struct d3d9_util_vertex *vertex, float x, float y, float error_margin)
{
    return vertex->x >= x - error_margin && vertex->x <= x + error_margin &&
        vertex->y >= y - error_margin && vertex->y <= y + error_margin;
}

static bool iidxhook_d3d9_bb_scale_hd_match_2d_plane(
    struct d3d9_util_vertex *vertices,
    float origin_x,
    float origin_y,
    float size_x,
    float size_y,
    float error_margin)
{
    return
        // top left
        iidxhook_d3d9_bb_scale_hd_in_range_of_2d_coords(
            &vertices[0], origin_x, origin_y, error_margin) &&
        // top right
        iidxhook_d3d9_bb_scale_hd_in_range_of_2d_coords(
            &vertices[1], origin_x + size_x, origin_y, error_margin) &&
        // bottom right
        iidxhook_d3d9_bb_scale_hd_in_range_of_2d_coords(
            &vertices[2], origin_x + size_x, origin_y + size_y, error_margin) &&
        // bottom left
        iidxhook_d3d9_bb_scale_hd_in_range_of_2d_coords(
            &vertices[3], origin_x, origin_y + size_y, error_margin);
}

static void iidxhook_d3d9_bb_scale_hd_set_plane(
    struct d3d9_util_vertex *vertices,
    float origin_x,
    float origin_y,
    float size_x,
    float size_y)
{
    // top left
    vertices[0].x = origin_x;
    vertices[0].y = origin_y;
    // top right
    vertices[1].x = origin_x + size_x;
    vertices[1].y = origin_y;
    // bottom right
    vertices[2].x = origin_x + size_x;
    vertices[2].y = origin_y + size_y;
    // bottom left
    vertices[3].x = origin_x;
    vertices[3].y = origin_y + size_y;
}

HRESULT iidxhook_d3d9_bb_scale_hd_d3d9_irp_handler(struct hook_d3d9_irp *irp)
{
    log_assert(irp);

    if (!iidxhook_d3d9_bb_scale_initialized) {
        return hook_d3d9_irp_invoke_next(irp);
    }

    if (irp->op == HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE) {
        // Scale the backbuffer accordingly
        // The game renders to an intermediate texture by default to provide the
        // SD vs. HD feature already
        irp->args.ctx_create_device.pp->BackBufferWidth =
            iidxhook_d3d9_bb_scale_hd_width;
        irp->args.ctx_create_device.pp->BackBufferHeight =
            iidxhook_d3d9_bb_scale_hd_height;

        return hook_d3d9_irp_invoke_next(irp);
    } else if (irp->op == HOOK_D3D9_IRP_OP_DEV_DRAW_PRIMITIVE_UP) {
        if (irp->args.dev_draw_primitive_up.primitive_type ==
                D3DPT_TRIANGLEFAN &&
            irp->args.dev_draw_primitive_up.primitive_count == 2 &&
            irp->args.dev_draw_primitive_up.stride == 28) {

            struct d3d9_util_vertex *vertices =
                (struct d3d9_util_vertex *)
                    irp->args.dev_draw_primitive_up.data;

            // The engine draws everything to a separate texture as the primary
            // render target Depending on the game mode selected, SD vs. HD/HD*,
            // the texture's size is either 640x480 or 1280x720 After drawing
            // the scene is finished (after EndScene), the game blends the
            // texture with the game's rendered output to the actual frame
            // buffer
            if (iidxhook_d3d9_bb_scale_hd_match_2d_plane(
                    vertices, 0, 0, 640, 480, 1.0) ||
                iidxhook_d3d9_bb_scale_hd_match_2d_plane(
                    vertices, 0, 0, 1280, 720, 1.0)) {
                // Note: Original plane is not at origin 0.0/0.0?
                // Example for SD plane:
                // -0.500000/-0.500000 639.500000/-0.500000
                // 639.500000/479.500000 -0.500000/479.500000 Patch to set it to
                // 0.0/0.0

                // Scale the 2D output plane primitive to the right resolution
                // (upscale or downscale) The render parameter for the blending
                // operation will automatically scale the selected source
                // texture, the intermediate render buffer with the game's final
                // frame, to the actual frame buffer once the DrawPrimitiveUP
                // call completed
                iidxhook_d3d9_bb_scale_hd_set_plane(
                    vertices,
                    0.0,
                    0.0,
                    iidxhook_d3d9_bb_scale_hd_width,
                    iidxhook_d3d9_bb_scale_hd_height);
            }
        }

        return hook_d3d9_irp_invoke_next(irp);
    } else if (irp->op == HOOK_D3D9_IRP_OP_DEV_RESET) {
        // Gets called when changing the "definition type" in the operator
        // menu. needs to be patched to keep the right back buffer size
        irp->args.dev_reset.pp->BackBufferWidth =
            iidxhook_d3d9_bb_scale_hd_width;
        irp->args.dev_reset.pp->BackBufferHeight =
            iidxhook_d3d9_bb_scale_hd_height;

        return hook_d3d9_irp_invoke_next(irp);
    } else if (irp->op == HOOK_D3D9_IRP_OP_DEV_SET_SCISSOR_RECT) {
        // Blending to the frame buffer applies scissors which needs to be
        // patched to match the final frame buffer's size
        RECT *rect = (RECT *) irp->args.dev_set_scissor_rect.pRect;

        // Only adjust scissoring when applied to full frame
        // Otherwise, this also scissors various UI elements
        if (rect->left == 0 && rect->left == 0 && rect->right == 1280 &&
            rect->bottom == 720) {
            rect->right = iidxhook_d3d9_bb_scale_hd_width;
            rect->bottom = iidxhook_d3d9_bb_scale_hd_height;
        }

        return hook_d3d9_irp_invoke_next(irp);
    } else {
        return hook_d3d9_irp_invoke_next(irp);
    }
}

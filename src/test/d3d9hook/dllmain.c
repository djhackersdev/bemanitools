#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "core/log-bt-ext.h"
#include "core/log-bt.h"
#include "core/log-sink-std.h"

#include "iface-core/log.h"

#include "hook/d3d9.h"

#include "test/check.h"

#define debug_print(...) fprintf(stderr, __VA_ARGS__)

static HRESULT my_d3d9_handler(struct hook_d3d9_irp *irp);

static const hook_d3d9_irp_handler_t d3d9_handlers[] = {my_d3d9_handler};

static HRESULT my_d3d9_handler(struct hook_d3d9_irp *irp)
{
    HRESULT hr;
    enum hook_d3d9_irp_op pre_op;

    debug_print("my_d3d9_handler dispatch: %d\n", irp->op);

    // Important: All parameter checks must align with the parameters from the
    // test executable

    // Pre-real invoke checks
    switch (irp->op) {
        case HOOK_D3D9_IRP_OP_INVALID:
            exit(EXIT_FAILURE);
            break;

        case HOOK_D3D9_IRP_OP_CTX_CREATE:
            debug_print("Pre HOOK_D3D9_IRP_OP_CTX_CREATE\n");

            check_int_eq(irp->args.ctx_create.sdk_ver, D3D_SDK_VERSION);
            check_null(irp->args.ctx_create.ctx);

            break;

        case HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES:
            debug_print("Pre HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES\n");

            check_null(irp->args.enum_display_devices.dev_name);
            check_int_eq(irp->args.enum_display_devices.dev_num, 0);
            check_non_null(irp->args.enum_display_devices.info);
            check_int_eq(
                irp->args.enum_display_devices.flags,
                EDD_GET_DEVICE_INTERFACE_NAME);

            break;

        case HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX:
            debug_print("Pre HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX\n");

            // Cannot test because wine is also using this call causing the
            // tests to fail check_int_eq(irp->args.create_window_ex.ex_style,
            // 0); check_str_eq(irp->args.create_window_ex.class_name,
            // "d3d9hook"); check_str_eq(irp->args.create_window_ex.window_name,
            // "d3d9hook test"); check_int_eq(irp->args.create_window_ex.style,
            // WS_OVERLAPPEDWINDOW); check_int_eq(irp->args.create_window_ex.x,
            // 0); check_int_eq(irp->args.create_window_ex.y, 0);
            // check_int_eq(irp->args.create_window_ex.width, 640);
            // check_int_eq(irp->args.create_window_ex.height, 480);
            // check_null(irp->args.create_window_ex.wnd_parent);
            // check_null(irp->args.create_window_ex.menu);
            // check_null(irp->args.create_window_ex.instance);
            // check_null(irp->args.create_window_ex.param);
            // check_null(irp->args.create_window_ex.result);

            break;

        case HOOK_D3D9_IRP_OP_GET_CLIENT_RECT:
            debug_print("Pre HOOK_D3D9_IRP_OP_GET_CLIENT_RECT\n");

            // Cannot test because wine is also using this call causing the
            // tests to fail

            break;

        case HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE:
            debug_print("Pre HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE\n");

            check_non_null(irp->args.ctx_create_device.self);
            check_int_eq(
                irp->args.ctx_create_device.adapter, D3DADAPTER_DEFAULT);
            check_int_eq(irp->args.ctx_create_device.type, D3DDEVTYPE_HAL);
            check_non_null(irp->args.ctx_create_device.hwnd);
            check_int_eq(
                irp->args.ctx_create_device.flags,
                D3DCREATE_HARDWARE_VERTEXPROCESSING);
            check_non_null(irp->args.ctx_create_device.pp);
            check_non_null(irp->args.ctx_create_device.pdev);

            break;

        case HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE:
            debug_print("Pre HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE\n");

            check_non_null(irp->args.dev_create_texture.self);
            check_int_eq(irp->args.dev_create_texture.width, 320);
            check_int_eq(irp->args.dev_create_texture.height, 240);
            check_int_eq(irp->args.dev_create_texture.levels, 0);
            check_int_eq(irp->args.dev_create_texture.usage, 0);
            check_int_eq(irp->args.dev_create_texture.format, D3DFMT_A8R8G8B8);
            check_int_eq(irp->args.dev_create_texture.pool, D3DPOOL_DEFAULT);
            check_non_null(irp->args.dev_create_texture.texture);
            check_null(irp->args.dev_create_texture.shared_handle);

            break;

        case HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE:
            debug_print("Pre HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE\n");

            check_non_null(irp->args.dev_begin_scene.self);

            break;

        case HOOK_D3D9_IRP_OP_DEV_END_SCENE:
            debug_print("Pre HOOK_D3D9_IRP_OP_DEV_END_SCENE\n");

            check_non_null(irp->args.dev_end_scene.self);

            break;

        case HOOK_D3D9_IRP_OP_DEV_PRESENT:
            debug_print("Pre HOOK_D3D9_IRP_OP_DEV_PRESENT\n");

            check_non_null(irp->args.dev_present.self);
            check_null(irp->args.dev_present.source_rect);
            check_null(irp->args.dev_present.dest_rect);
            check_null(irp->args.dev_present.dest_window_override);
            check_null(irp->args.dev_present.dirty_region);

            break;

        case HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE:
            debug_print("Pre HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE\n");

            check_non_null(irp->args.dev_set_render_state.self);
            check_int_eq(irp->args.dev_set_render_state.state, D3DRS_FILLMODE);
            check_int_eq(irp->args.dev_set_render_state.value, D3DFILL_POINT);

            break;

        default:
            debug_print("Unhandled op code: %d\n", irp->op);
            check_fail_msg("Unhandled op code.");
            break;
    }

    pre_op = irp->op;

    hr = hook_d3d9_irp_invoke_next(irp);

    if (hr != S_OK) {
        debug_print("HRESULT of op %d: %lX\n", irp->op, hr);
    } else {
        check_int_eq(irp->op, pre_op);

        // Post real invoke checks
        switch (irp->op) {
            case HOOK_D3D9_IRP_OP_INVALID:
                exit(EXIT_FAILURE);
                break;

            case HOOK_D3D9_IRP_OP_CTX_CREATE:
                debug_print("Post HOOK_D3D9_IRP_OP_CTX_CREATE\n");

                check_int_eq(irp->args.ctx_create.sdk_ver, D3D_SDK_VERSION);
                check_non_null(irp->args.ctx_create.ctx);

                break;

            case HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES:
                debug_print("Post HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES\n");

                check_null(irp->args.enum_display_devices.dev_name);
                check_int_eq(irp->args.enum_display_devices.dev_num, 0);
                check_non_null(irp->args.enum_display_devices.info);
                check_int_eq(
                    irp->args.enum_display_devices.flags,
                    EDD_GET_DEVICE_INTERFACE_NAME);

                break;

            case HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX:
                debug_print("Post HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX\n");

                // Cannot test because wine is also using this call causing the
                // tests to fail
                // check_int_eq(irp->args.create_window_ex.ex_style, 0);
                // check_str_eq(irp->args.create_window_ex.class_name,
                // "d3d9hook");
                // check_str_eq(irp->args.create_window_ex.window_name,
                // "d3d9hook test");
                // check_int_eq(irp->args.create_window_ex.style,
                // WS_OVERLAPPEDWINDOW);
                // check_int_eq(irp->args.create_window_ex.x, 0);
                // check_int_eq(irp->args.create_window_ex.y, 0);
                // check_int_eq(irp->args.create_window_ex.width, 640);
                // check_int_eq(irp->args.create_window_ex.height, 480);
                // check_null(irp->args.create_window_ex.wnd_parent);
                // check_null(irp->args.create_window_ex.menu);
                // check_null(irp->args.create_window_ex.instance);
                // check_null(irp->args.create_window_ex.param);
                // check_non_null(irp->args.create_window_ex.result);

                break;

            case HOOK_D3D9_IRP_OP_GET_CLIENT_RECT:
                debug_print("Post HOOK_D3D9_IRP_OP_GET_CLIENT_RECT\n");

                // Cannot test because wine is also using this call causing the
                // tests to fail

                break;

            case HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE:
                debug_print("Post HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE\n");

                check_non_null(irp->args.ctx_create_device.self);
                check_int_eq(
                    irp->args.ctx_create_device.adapter, D3DADAPTER_DEFAULT);
                check_int_eq(irp->args.ctx_create_device.type, D3DDEVTYPE_HAL);
                check_non_null(irp->args.ctx_create_device.hwnd);
                check_int_eq(
                    irp->args.ctx_create_device.flags,
                    D3DCREATE_HARDWARE_VERTEXPROCESSING);
                check_non_null(irp->args.ctx_create_device.pp);
                check_non_null(irp->args.ctx_create_device.pdev);

                break;

            case HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE:
                debug_print("Post HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE\n");

                check_non_null(irp->args.dev_create_texture.self);
                check_int_eq(irp->args.dev_create_texture.width, 320);
                check_int_eq(irp->args.dev_create_texture.height, 240);
                check_int_eq(irp->args.dev_create_texture.levels, 0);
                check_int_eq(irp->args.dev_create_texture.usage, 0);
                check_int_eq(
                    irp->args.dev_create_texture.format, D3DFMT_A8R8G8B8);
                check_int_eq(
                    irp->args.dev_create_texture.pool, D3DPOOL_DEFAULT);
                check_non_null(irp->args.dev_create_texture.texture);
                check_null(irp->args.dev_create_texture.shared_handle);

                break;

            case HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE:
                debug_print("Pre HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE\n");

                check_non_null(irp->args.dev_begin_scene.self);

                break;

            case HOOK_D3D9_IRP_OP_DEV_END_SCENE:
                debug_print("Pre HOOK_D3D9_IRP_OP_DEV_END_SCENE\n");

                check_non_null(irp->args.dev_end_scene.self);

                break;

            case HOOK_D3D9_IRP_OP_DEV_PRESENT:
                debug_print("Post HOOK_D3D9_IRP_OP_DEV_PRESENT\n");

                check_non_null(irp->args.dev_present.self);
                check_null(irp->args.dev_present.source_rect);
                check_null(irp->args.dev_present.dest_rect);
                check_null(irp->args.dev_present.dest_window_override);
                check_null(irp->args.dev_present.dirty_region);

                break;

            case HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE:
                debug_print("Post HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE\n");

                check_non_null(irp->args.dev_set_render_state.self);
                check_int_eq(
                    irp->args.dev_set_render_state.state, D3DRS_FILLMODE);
                check_int_eq(
                    irp->args.dev_set_render_state.value, D3DFILL_POINT);

                break;

            default:
                debug_print("Unhandled op code: %d\n", irp->op);
                check_fail_msg("Unhandled op code.");
                break;
        }
    }

    return hr;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        core_log_bt_core_api_set();

        core_log_bt_ext_init_with_stderr();
        core_log_bt_level_set(CORE_LOG_BT_LOG_LEVEL_MISC);

        debug_print("Initializing d3d9 hook module...\n");

        hook_d3d9_init(d3d9_handlers, lengthof(d3d9_handlers));
    }

    return TRUE;
}
#ifndef HOOK_D3D9_H
#define HOOK_D3D9_H

/**
 * This module hooks the d3d9 API and abstracts it using IRPs (like iohook).
 * It allows hiding the COM stuff which is required to be handled when
 * interacting with d3d9. Furthermore, this module supports chaining of multiple
 * handlers and more flexible and structured dispatching on the available
 * operations (more can be added when required, obviously).
 *
 * @author icex2
 */

#include <d3d9.h>
#include <windows.h>

#include <stddef.h>
#include <stdint.h>

/**
 * D3D9 ops (currently) supported by this hook module.
 */
enum hook_d3d9_irp_op {
    HOOK_D3D9_IRP_OP_INVALID = 0,
    HOOK_D3D9_IRP_OP_CTX_CREATE = 1,
    HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES = 2,
    HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX = 3,
    HOOK_D3D9_IRP_OP_GET_CLIENT_RECT = 4,
    HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE = 5,
    HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE = 6,
    HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE = 7,
    HOOK_D3D9_IRP_OP_DEV_END_SCENE = 8,
    HOOK_D3D9_IRP_OP_DEV_PRESENT = 9,
    HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE = 10,
    HOOK_D3D9_IRP_OP_DEV_DRAW_PRIMITIVE_UP = 11,
};

/**
 * Main irp structure which gets passed around as a parameter to chain multiple
 * handler calls and avoiding one handler per request/function.
 */
struct hook_d3d9_irp {
    enum hook_d3d9_irp_op op;

    union {
        /**
         * Params of Direct3DCreate9.
         */
        struct {
            UINT sdk_ver;
            // Return value of call
            IDirect3D9 *ctx;
        } ctx_create;

        /**
         * Params of EnumDisplayDevicesA.
         */
        struct {
            const char *dev_name;
            DWORD dev_num;
            DISPLAY_DEVICEA *info;
            DWORD flags;
        } enum_display_devices;

        /**
         * Params of CreateWindowExA.
         */
        struct {
            DWORD ex_style;
            LPCSTR class_name;
            LPCSTR window_name;
            DWORD style;
            int x;
            int y;
            int width;
            int height;
            HWND wnd_parent;
            HMENU menu;
            HINSTANCE instance;
            LPVOID param;
            HWND result;
        } create_window_ex;

        /**
         * Params of GetClientRect.
         */
        struct {
            HWND wnd;
            LPRECT rect;
        } get_client_rect;

        /**
         * Params of IDirect3D9_CreateDevice.
         */
        struct {
            IDirect3D9 *self;
            UINT adapter;
            D3DDEVTYPE type;
            HWND hwnd;
            DWORD flags;
            D3DPRESENT_PARAMETERS *pp;
            IDirect3DDevice9 **pdev;
        } ctx_create_device;

        /**
         * Params of IDirect3DDevice9_CreateTexture.
         */
        struct {
            IDirect3DDevice9 *self;
            UINT width;
            UINT height;
            UINT levels;
            DWORD usage;
            D3DFORMAT format;
            D3DPOOL pool;
            IDirect3DTexture9 **texture;
            HANDLE *shared_handle;
        } dev_create_texture;

        /**
         * Params of IDirect3DDevice9_BeginScene.
         */
        struct {
            IDirect3DDevice9 *self;
        } dev_begin_scene;

        /**
         * Params of IDirect3DDevice9_EndScene.
         */
        struct {
            IDirect3DDevice9 *self;
        } dev_end_scene;

        /**
         * Params of IDirect3DDevice9_Present.
         */
        struct {
            IDirect3DDevice9 *self;
            CONST RECT *source_rect;
            CONST RECT *dest_rect;
            HWND dest_window_override;
            CONST RGNDATA *dirty_region;
        } dev_present;

        /**
         * Params of IDirect3DDevice9_SetRenderState.
         */
        struct {
            IDirect3DDevice9 *self;
            D3DRENDERSTATETYPE state;
            DWORD value;
        } dev_set_render_state;

        /**
         * Params of IDirect3DDevice9_DrawPrimitiveUP.
         */
        struct {
            IDirect3DDevice9 *self;
            D3DPRIMITIVETYPE primitive_type;
            UINT primitive_count;
            const void *data;
            UINT stride;
        } dev_draw_primitive_up;
    } args;

    size_t next_handler;
};

/**
 * D3D9 IRP handler function typedef.
 */
typedef HRESULT (*hook_d3d9_irp_handler_t)(struct hook_d3d9_irp *irp);

/**
 * Initialize the d3d9 hook module.
 *
 * @param handlers The handlers to install which will be called once one of the
 * hooked functions is called.
 * @param nhandlers Number of handlers to install (length of provided array
 * above).
 */
void hook_d3d9_init(const hook_d3d9_irp_handler_t *handlers, size_t nhandlers);

/**
 * Main invoke next function.
 *
 * Call this from your handlers if you are done handling and to pass on the IRP
 * to the further installed IRP handlers. At the end of the chain, calls the
 * real function.
 *
 * @param irp Pointer to a valid IRP.
 * @return S_OK on success, error otherwise (depending on the irp op).
 */
HRESULT hook_d3d9_irp_invoke_next(struct hook_d3d9_irp *irp);

#endif
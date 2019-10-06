#define LOG_MODULE "hook-d3d9"

#include "hook/com-proxy.h"
#include "hook/d3d9.h"
#include "hook/table.h"

#include "util/log.h"

/* ------------------------------------------------------------------------------------------------------------------ */

static IDirect3D9* (STDCALL* real_Direct3DCreate9)(UINT sdk_ver);

static BOOL (STDCALL *real_EnumDisplayDevicesA)(
        const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info,
        DWORD flags);

static HWND (STDCALL *real_CreateWindowExA)(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam);

static BOOL (STDCALL* real_GetClientRect)(HWND hWnd, LPRECT lpRect);

static HRESULT (STDCALL* real_CreateDevice)(IDirect3D9 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd, DWORD flags,
        D3DPRESENT_PARAMETERS *pp, IDirect3DDevice9 **pdev);

static HRESULT (STDCALL* real_CreateTexture)(IDirect3DDevice9* self, UINT width, UINT height, UINT levels, DWORD usage,
        D3DFORMAT format, D3DPOOL pool, IDirect3DTexture9** texture, HANDLE* shared_handle);

static HRESULT (STDCALL* real_BeginScene)(IDirect3DDevice9* self);

static HRESULT (STDCALL* real_EndScene)(IDirect3DDevice9* self);

static HRESULT (STDCALL* real_Present)(IDirect3DDevice9* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);

static HRESULT (STDCALL* real_SetRenderState)(IDirect3DDevice9* self,
        D3DRENDERSTATETYPE State, DWORD Value);

static HRESULT (STDCALL *real_DrawPrimitiveUP)(
        IDirect3DDevice9* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride);

/* ------------------------------------------------------------------------------------------------------------------ */

static IDirect3D9* STDCALL my_Direct3DCreate9(UINT sdk_ver);

static BOOL STDCALL my_EnumDisplayDevicesA(const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info, DWORD flags);

static HWND STDCALL my_CreateWindowExA(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam);

static BOOL STDCALL my_GetClientRect(HWND hWnd, LPRECT lpRect);

static HRESULT STDCALL my_CreateDevice(IDirect3D9 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd, DWORD flags,
        D3DPRESENT_PARAMETERS *pp, IDirect3DDevice9 **pdev);

static HRESULT STDCALL my_CreateTexture(IDirect3DDevice9* self, UINT width, UINT height, UINT levels, DWORD usage,
        D3DFORMAT format, D3DPOOL pool, IDirect3DTexture9** texture, HANDLE* shared_handle);

static HRESULT STDCALL my_BeginScene(IDirect3DDevice9* self);

static HRESULT STDCALL my_EndScene(IDirect3DDevice9* self);

static HRESULT STDCALL my_Present(IDirect3DDevice9* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion);

static HRESULT STDCALL my_SetRenderState(IDirect3DDevice9* self,
        D3DRENDERSTATETYPE State, DWORD Value);

static HRESULT STDCALL my_DrawPrimitiveUP(
        IDirect3DDevice9* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride);

/* ------------------------------------------------------------------------------------------------------------------ */

static const struct hook_symbol hook_d3d9_hook_syms[] = {
    {
        .name   = "Direct3DCreate9",
        .patch  = my_Direct3DCreate9,
        .link   = (void **) &real_Direct3DCreate9
    },
};

static const struct hook_symbol hook_d3d9_hook_user32_syms[] = {
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

/* ------------------------------------------------------------------------------------------------------------------ */

static HRESULT hook_d3d9_irp_handler_real_invalid(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_ctx_create(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_enum_display_devices(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_create_window_ex(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_get_client_rect(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_dev_create_device(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_dev_create_texture(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_dev_begin_scene(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_dev_end_scene(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_dev_present(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_dev_set_render_state(struct hook_d3d9_irp* irp);

static HRESULT hook_d3d9_irp_handler_real_dev_draw_primitive_up(struct hook_d3d9_irp* irp);

/* ------------------------------------------------------------------------------------------------------------------ */

static const hook_d3d9_irp_handler_t hook_d3d9_irp_real_handlers[] = {
    [HOOK_D3D9_IRP_OP_INVALID] = hook_d3d9_irp_handler_real_invalid,
    [HOOK_D3D9_IRP_OP_CTX_CREATE] = hook_d3d9_irp_handler_real_ctx_create,
    [HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES] = hook_d3d9_irp_handler_real_enum_display_devices,
    [HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX] = hook_d3d9_irp_handler_real_create_window_ex,
    [HOOK_D3D9_IRP_OP_GET_CLIENT_RECT] = hook_d3d9_irp_handler_real_get_client_rect,
    [HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE] = hook_d3d9_irp_handler_real_dev_create_device,
    [HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE] = hook_d3d9_irp_handler_real_dev_create_texture,
    [HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE] = hook_d3d9_irp_handler_real_dev_begin_scene,
    [HOOK_D3D9_IRP_OP_DEV_END_SCENE] = hook_d3d9_irp_handler_real_dev_end_scene,
    [HOOK_D3D9_IRP_OP_DEV_PRESENT] = hook_d3d9_irp_handler_real_dev_present,
    [HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE] = hook_d3d9_irp_handler_real_dev_set_render_state,
    [HOOK_D3D9_IRP_OP_DEV_DRAW_PRIMITIVE_UP] = hook_d3d9_irp_handler_real_dev_draw_primitive_up,
};

static const hook_d3d9_irp_handler_t* hook_d3d9_handlers;
static size_t hook_d3d9_nhandlers;

/* ------------------------------------------------------------------------------------------------------------------ */

static IDirect3D9* STDCALL my_Direct3DCreate9(UINT sdk_ver)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_CTX_CREATE;
    irp.args.ctx_create.sdk_ver = sdk_ver;
    irp.args.ctx_create.ctx = NULL;

    hr = hook_d3d9_irp_invoke_next(&irp);

    if (hr == S_OK && irp.args.ctx_create.ctx) {
        return irp.args.ctx_create.ctx;
    } else {
        return NULL;
    }
}

static BOOL STDCALL my_EnumDisplayDevicesA(const char *dev_name, DWORD dev_num, DISPLAY_DEVICEA *info, DWORD flags)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_ENUM_DISPLAY_DEVICES;
    irp.args.enum_display_devices.dev_name = dev_name;
    irp.args.enum_display_devices.dev_num = dev_num;
    irp.args.enum_display_devices.info = info;
    irp.args.enum_display_devices.flags = flags;

    hr = hook_d3d9_irp_invoke_next(&irp);

    if (hr == S_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static HWND STDCALL my_CreateWindowExA(
        DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
        int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
        HINSTANCE hInstance, LPVOID lpParam)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_CREATE_WINDOW_EX;
    irp.args.create_window_ex.ex_style = dwExStyle;
    irp.args.create_window_ex.class_name = lpClassName;
    irp.args.create_window_ex.window_name = lpWindowName;
    irp.args.create_window_ex.style = dwStyle;
    irp.args.create_window_ex.x = X;
    irp.args.create_window_ex.y = Y;
    irp.args.create_window_ex.width = nWidth;
    irp.args.create_window_ex.height = nHeight;
    irp.args.create_window_ex.wnd_parent = hWndParent;
    irp.args.create_window_ex.menu = hMenu;
    irp.args.create_window_ex.instance = hInstance;
    irp.args.create_window_ex.param = lpParam;
    irp.args.create_window_ex.result = NULL;

    hr = hook_d3d9_irp_invoke_next(&irp);

    if (hr == S_OK && irp.args.create_window_ex.result != NULL) {
        return irp.args.create_window_ex.result;
    } else {
        return NULL;
    }   
}

static BOOL STDCALL my_GetClientRect(HWND hWnd, LPRECT lpRect)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_GET_CLIENT_RECT;
    irp.args.get_client_rect.wnd = hWnd;
    irp.args.get_client_rect.rect = lpRect;

    hr = hook_d3d9_irp_invoke_next(&irp);

    if (hr == S_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static HRESULT STDCALL my_CreateDevice(
    IDirect3D9 *self, UINT adapter, D3DDEVTYPE type, HWND hwnd, DWORD flags,
    D3DPRESENT_PARAMETERS *pp, IDirect3DDevice9 **pdev)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE;
    irp.args.ctx_create_device.self = self;
    irp.args.ctx_create_device.adapter = adapter;
    irp.args.ctx_create_device.type = type;
    irp.args.ctx_create_device.hwnd = hwnd;
    irp.args.ctx_create_device.flags = flags;
    irp.args.ctx_create_device.pp = pp;
    irp.args.ctx_create_device.pdev = pdev;

    hr = hook_d3d9_irp_invoke_next(&irp);

    return hr;
}

static HRESULT STDCALL my_CreateTexture(IDirect3DDevice9* self, UINT width, UINT height, UINT levels, DWORD usage,
        D3DFORMAT format, D3DPOOL pool, IDirect3DTexture9** texture, HANDLE* shared_handle)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_DEV_CREATE_TEXTURE;
    irp.args.dev_create_texture.self = self;
    irp.args.dev_create_texture.width = width;
    irp.args.dev_create_texture.height = height;
    irp.args.dev_create_texture.levels = levels;
    irp.args.dev_create_texture.usage = usage;
    irp.args.dev_create_texture.format = format;
    irp.args.dev_create_texture.pool = pool;
    irp.args.dev_create_texture.texture = texture;
    irp.args.dev_create_texture.shared_handle = shared_handle;

    hr = hook_d3d9_irp_invoke_next(&irp);

    return hr;
}

static HRESULT STDCALL my_BeginScene(IDirect3DDevice9* self)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE;
    irp.args.dev_begin_scene.self = self;

    hr = hook_d3d9_irp_invoke_next(&irp);

    return hr;
}

static HRESULT STDCALL my_EndScene(IDirect3DDevice9* self)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_DEV_END_SCENE;
    irp.args.dev_end_scene.self = self;

    hr = hook_d3d9_irp_invoke_next(&irp);

    return hr;
}

static HRESULT STDCALL my_Present(IDirect3DDevice9* self,
        CONST RECT *pSourceRect, CONST RECT *pDestRect,
        HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_DEV_PRESENT;
    irp.args.dev_present.self = self;
    irp.args.dev_present.source_rect = pSourceRect;
    irp.args.dev_present.dest_rect = pDestRect;
    irp.args.dev_present.dest_window_override = hDestWindowOverride;
    irp.args.dev_present.dirty_region = pDirtyRegion;

    hr = hook_d3d9_irp_invoke_next(&irp);

    return hr;
}

static HRESULT STDCALL my_SetRenderState(IDirect3DDevice9* self,
        D3DRENDERSTATETYPE State, DWORD Value)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_DEV_SET_RENDER_STATE;
    irp.args.dev_set_render_state.self = self;
    irp.args.dev_set_render_state.state = State;
    irp.args.dev_set_render_state.value = Value;

    hr = hook_d3d9_irp_invoke_next(&irp);

    return hr;   
}

static HRESULT STDCALL my_DrawPrimitiveUP(
        IDirect3DDevice9* self, D3DPRIMITIVETYPE primitive_type,
        UINT primitive_count, const void *data, UINT stride)
{
    struct hook_d3d9_irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));

    irp.op = HOOK_D3D9_IRP_OP_DEV_DRAW_PRIMITIVE_UP;
    irp.args.dev_draw_primitive_up.self = self;
    irp.args.dev_draw_primitive_up.primitive_type = primitive_type;
    irp.args.dev_draw_primitive_up.primitive_count = primitive_count;
    irp.args.dev_draw_primitive_up.data = data;
    irp.args.dev_draw_primitive_up.stride = stride;

    hr = hook_d3d9_irp_invoke_next(&irp);

    return hr;       
}

/* ------------------------------------------------------------------------------------------------------------------ */

static HRESULT hook_d3d9_irp_handler_invoke_real(struct hook_d3d9_irp* irp)
{
    hook_d3d9_irp_handler_t handler;

    log_assert(irp != NULL);
    log_assert(irp->op < lengthof(hook_d3d9_irp_real_handlers));

    handler = hook_d3d9_irp_real_handlers[irp->op];

    log_assert(handler != NULL);

    return handler(irp);
}

static HRESULT hook_d3d9_irp_handler_real_invalid(struct hook_d3d9_irp* irp)
{
    log_fatal("Called invalid handler");

    return E_FAIL;
}

static HRESULT hook_d3d9_irp_handler_real_ctx_create(struct hook_d3d9_irp* irp)
{
    IDirect3D9* api;
    IDirect3D9Vtbl* api_vtbl;
    struct com_proxy* api_proxy;

    log_assert(irp);

    api = real_Direct3DCreate9(irp->args.ctx_create.sdk_ver);

    if (api == NULL) {
        return E_FAIL;
    }

    api_proxy = com_proxy_wrap(api, sizeof(*api->lpVtbl));
    api_vtbl = api_proxy->vptr;

    real_CreateDevice = api_vtbl->CreateDevice;
    api_vtbl->CreateDevice = my_CreateDevice;

    irp->args.ctx_create.ctx = (IDirect3D9*) api_proxy;

    return S_OK;   
}

static HRESULT hook_d3d9_irp_handler_real_enum_display_devices(struct hook_d3d9_irp* irp)
{
    BOOL res;

    log_assert(irp);

    res = real_EnumDisplayDevicesA(
            irp->args.enum_display_devices.dev_name,
            irp->args.enum_display_devices.dev_num,
            irp->args.enum_display_devices.info,
            irp->args.enum_display_devices.flags);

    if (res == TRUE) {
        return S_OK;
    } else {
        return E_FAIL;
    }
}

static HRESULT hook_d3d9_irp_handler_real_create_window_ex(struct hook_d3d9_irp* irp)
{
    log_assert(irp);

    irp->args.create_window_ex.result = real_CreateWindowExA(
            irp->args.create_window_ex.ex_style,
            irp->args.create_window_ex.class_name,
            irp->args.create_window_ex.window_name,
            irp->args.create_window_ex.style,
            irp->args.create_window_ex.x,
            irp->args.create_window_ex.y,
            irp->args.create_window_ex.width,
            irp->args.create_window_ex.height,
            irp->args.create_window_ex.wnd_parent,
            irp->args.create_window_ex.menu,
            irp->args.create_window_ex.instance,
            irp->args.create_window_ex.param);

    if (irp->args.create_window_ex.result) {
        return S_OK;
    } else {
        return E_FAIL;
    }
}

static HRESULT hook_d3d9_irp_handler_real_get_client_rect(struct hook_d3d9_irp* irp)
{
    BOOL res;

    log_assert(irp);

    res = real_GetClientRect(
            irp->args.get_client_rect.wnd,
            irp->args.get_client_rect.rect);

    if (res == TRUE) {
        return S_OK;
    } else {
        return E_FAIL;
    }
}

static HRESULT hook_d3d9_irp_handler_real_dev_create_device(struct hook_d3d9_irp* irp)
{
    HRESULT hr;
    IDirect3DDevice9* api;
    IDirect3DDevice9Vtbl* api_vtbl;
    struct com_proxy *api_proxy;

    log_assert(irp);

    hr = real_CreateDevice(
            irp->args.ctx_create_device.self,
            irp->args.ctx_create_device.adapter,
            irp->args.ctx_create_device.type,
            irp->args.ctx_create_device.hwnd,
            irp->args.ctx_create_device.flags,
            irp->args.ctx_create_device.pp,
            irp->args.ctx_create_device.pdev);

    if (hr != S_OK) {
        return hr;
    }

    api = *irp->args.ctx_create_device.pdev;
    api_proxy = com_proxy_wrap(api, sizeof(*api->lpVtbl));
    api_vtbl = api_proxy->vptr;

    real_CreateTexture = api_vtbl->CreateTexture;
    api_vtbl->CreateTexture = my_CreateTexture;

    real_BeginScene = api_vtbl->BeginScene;
    api_vtbl->BeginScene = my_BeginScene;

    real_EndScene = api_vtbl->EndScene;
    api_vtbl->EndScene = my_EndScene;

    real_Present = api_vtbl->Present;
    api_vtbl->Present = my_Present;

    real_SetRenderState = api_vtbl->SetRenderState;
    api_vtbl->SetRenderState = my_SetRenderState;

    real_DrawPrimitiveUP = api_vtbl->DrawPrimitiveUP;
    api_vtbl->DrawPrimitiveUP = my_DrawPrimitiveUP;

    *irp->args.ctx_create_device.pdev = (IDirect3DDevice9*) api_proxy;

    return hr;
}

static HRESULT hook_d3d9_irp_handler_real_dev_create_texture(struct hook_d3d9_irp* irp)
{
    log_assert(irp);

    return real_CreateTexture(
            irp->args.dev_create_texture.self,
            irp->args.dev_create_texture.width,
            irp->args.dev_create_texture.height,
            irp->args.dev_create_texture.levels,
            irp->args.dev_create_texture.usage,
            irp->args.dev_create_texture.format,
            irp->args.dev_create_texture.pool,
            irp->args.dev_create_texture.texture,
            irp->args.dev_create_texture.shared_handle);
}

static HRESULT hook_d3d9_irp_handler_real_dev_begin_scene(struct hook_d3d9_irp* irp)
{
    log_assert(irp);

    return real_BeginScene(irp->args.dev_begin_scene.self);
}

static HRESULT hook_d3d9_irp_handler_real_dev_end_scene(struct hook_d3d9_irp* irp)
{
    log_assert(irp);

    return real_EndScene(irp->args.dev_end_scene.self);
}

static HRESULT hook_d3d9_irp_handler_real_dev_present(struct hook_d3d9_irp* irp)
{
    log_assert(irp);

    return real_Present(
            irp->args.dev_present.self,
            irp->args.dev_present.source_rect,
            irp->args.dev_present.dest_rect,
            irp->args.dev_present.dest_window_override,
            irp->args.dev_present.dirty_region);
}

static HRESULT hook_d3d9_irp_handler_real_dev_set_render_state(struct hook_d3d9_irp* irp)
{
    log_assert(irp);

    return real_SetRenderState(
            irp->args.dev_set_render_state.self,
            irp->args.dev_set_render_state.state,
            irp->args.dev_set_render_state.value);
}

static HRESULT hook_d3d9_irp_handler_real_dev_draw_primitive_up(struct hook_d3d9_irp* irp)
{
    log_assert(irp);

    return real_DrawPrimitiveUP(
            irp->args.dev_draw_primitive_up.self,
            irp->args.dev_draw_primitive_up.primitive_type,
            irp->args.dev_draw_primitive_up.primitive_count,
            irp->args.dev_draw_primitive_up.data,
            irp->args.dev_draw_primitive_up.stride);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void hook_d3d9_init(const hook_d3d9_irp_handler_t *handlers, size_t nhandlers)
{
    log_assert(handlers);
    log_assert(nhandlers > 0);

    hook_d3d9_handlers = handlers;
    hook_d3d9_nhandlers = nhandlers;

    hook_table_apply(
            NULL,
            "d3d9.dll",
            hook_d3d9_hook_syms,
            lengthof(hook_d3d9_hook_syms));

    hook_table_apply(
            NULL,
            "user32.dll",
            hook_d3d9_hook_user32_syms,
            lengthof(hook_d3d9_hook_user32_syms));
}

HRESULT hook_d3d9_irp_invoke_next(struct hook_d3d9_irp* irp)
{
    hook_d3d9_irp_handler_t handler;
    HRESULT hr;

    log_assert(irp != NULL);
    log_assert(irp->next_handler <= hook_d3d9_nhandlers);

    if (irp->next_handler < hook_d3d9_nhandlers) {
        handler = hook_d3d9_handlers[irp->next_handler++];
        hr = handler(irp);

        if (FAILED(hr)) {
            irp->next_handler = (size_t) -1;
        }
    } else {
        irp->next_handler = (size_t) -1;
        hr = hook_d3d9_irp_handler_invoke_real(irp);
    }

    return hr;
}
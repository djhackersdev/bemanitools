#define LOG_MODULE "imgui-d3d9-hook"

#include "hook/d3d9.h"
#include "hook/table.h"

#include "imgui-bt/cimgui.h"
#include "imgui-bt/component.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

typedef LRESULT WINAPI (*RegisterClassA_t)(WNDCLASSA *lpwcx);
typedef LRESULT WINAPI (*WndProc_t)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI _imgui_d3d9_hook_RegisterClassA(WNDCLASSA *lpwcx);

static bool _imgui_d3d9_hook_initialized;
static ImGuiContext *_imgui_d3d9_hook_imgui_ctx;
static RegisterClassA_t _imgui_d3d9_hook_original_RegisterClassA;
static WndProc_t _imgui_d3d9_hook_original_wndproc;
static imgui_bt_component_t *_imgui_d3d9_hook_components;
static size_t _imgui_d3d9_hook_component_count;

static const struct hook_symbol _imgui_d3d9_hook_syms[] = {
    {.name = "RegisterClassA",
     .patch = _imgui_d3d9_hook_RegisterClassA,
     .link = (void **) &_imgui_d3d9_hook_original_RegisterClassA},
};

CIMGUI_API LRESULT igImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI _imgui_d3d9_hook_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;
    
    result = igImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

    if (result) {
        return result;
    }

    return _imgui_d3d9_hook_original_wndproc(hWnd, msg, wParam, lParam);
}

static LRESULT WINAPI _imgui_d3d9_hook_RegisterClassA(WNDCLASSA *lpwcx)
{
    log_info("Swapping out WndProc handler for wrapper handler in RegisterClassA %s", lpwcx->lpszClassName);

    _imgui_d3d9_hook_original_wndproc = lpwcx->lpfnWndProc;
    lpwcx->lpfnWndProc = _imgui_d3d9_hook_WndProc;

    return _imgui_d3d9_hook_original_RegisterClassA(lpwcx);
}

void imgui_d3d9_hook_init(const imgui_bt_component_t *components, size_t component_count)
{
    ImGuiIO *io;
    ImGuiStyle* style;

    log_assert(components);
    log_assert(component_count > 0);

    _imgui_d3d9_hook_components = (imgui_bt_component_t *) xmalloc(component_count * sizeof(imgui_bt_component_t));
    memcpy(_imgui_d3d9_hook_components, components, component_count * sizeof(imgui_bt_component_t));
    _imgui_d3d9_hook_component_count = component_count;

    _imgui_d3d9_hook_imgui_ctx = igCreateContext(NULL);
    io = igGetIO();

    io->ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard
        | ImGuiConfigFlags_DockingEnable
        | ImGuiConfigFlags_ViewportsEnable;

    io->MouseDrawCursor = true;
    io->IniFilename = NULL;
    io->FontAllowUserScaling = true;

    hook_table_apply(NULL, "user32.dll", _imgui_d3d9_hook_syms, lengthof(_imgui_d3d9_hook_syms));

    // Setup style
    style = igGetStyle();
    style->Colors[ImGuiCol_WindowBg] = (ImVec4){0.0f, 0.0f, 0.0f, 1.0f};
    style->Colors[ImGuiCol_PlotLines] = (ImVec4){1.0f, 1.0f, 0.0f, 1.0f};

    _imgui_d3d9_hook_initialized = true;
}

void imgui_d3d9_hook_fini()
{
    hook_table_revert(NULL, "user32.dll", _imgui_d3d9_hook_syms, lengthof(_imgui_d3d9_hook_syms));

    igImplDX9_Shutdown();
    igImplWin32_Shutdown();
    igDestroyContext(NULL);

    _imgui_d3d9_hook_initialized = false;
}

HRESULT imgui_hook_d3d9_irp_handler(struct hook_d3d9_irp *irp)
{
    HRESULT hr;

    log_assert(irp);

    if (!_imgui_d3d9_hook_initialized) {
        return hook_d3d9_irp_invoke_next(irp);
    }

    switch (irp->op) {
        case HOOK_D3D9_IRP_OP_CTX_CREATE_DEVICE:
            hr = hook_d3d9_irp_invoke_next(irp);

            if (hr == S_OK) {
                igImplWin32_Init(irp->args.ctx_create_device.hwnd);
                igImplDX9_Init(*irp->args.ctx_create_device.pdev);
            }

            return hr;
        
        case HOOK_D3D9_IRP_OP_DEV_BEGIN_SCENE:
            igImplDX9_NewFrame();
            igImplWin32_NewFrame();
            igNewFrame();

            for (size_t i = 0; i < _imgui_d3d9_hook_component_count; i++) {
                _imgui_d3d9_hook_components[i].frame_update(_imgui_d3d9_hook_imgui_ctx);
            }
        
            return hook_d3d9_irp_invoke_next(irp);

        case HOOK_D3D9_IRP_OP_DEV_END_SCENE:
            igRender();
            igImplDX9_RenderDrawData(igGetDrawData());

            return hook_d3d9_irp_invoke_next(irp);
        
        default:
            return hook_d3d9_irp_invoke_next(irp);
    }

    log_fatal("Illegal state");
}
#include "imgui_impl_dx9.h"
#include "imgui_internal.h"
#include "cimgui.h"

CIMGUI_API bool igImplDX9_Init(IDirect3DDevice9* device)
{
    return ImGui_ImplDX9_Init(device);
}

CIMGUI_API void igImplDX9_Shutdown()
{
    ImGui_ImplDX9_Shutdown();
}

CIMGUI_API void igImplDX9_NewFrame()
{
    ImGui_ImplDX9_NewFrame();
}

CIMGUI_API void igImplDX9_RenderDrawData(ImDrawData* draw_data)
{
    ImGui_ImplDX9_RenderDrawData(draw_data);
}

CIMGUI_API bool igImplDX9_CreateDeviceObjects()
{
    return ImGui_ImplDX9_CreateDeviceObjects();
}

CIMGUI_API void igImplDX9_InvalidateDeviceObjects()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}
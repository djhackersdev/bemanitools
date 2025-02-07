#ifndef CIMGUI_IMPL_DX9_INCLUDED
#define CIMGUI_IMPL_DX9_INCLUDED

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

typedef struct IDirect3DDevice9 IDirect3DDevice9;

CIMGUI_API bool igImplDX9_Init(IDirect3DDevice9* device);
CIMGUI_API void igImplDX9_Shutdown();
CIMGUI_API void igImplDX9_NewFrame();
CIMGUI_API void igImplDX9_RenderDrawData(ImDrawData* draw_data);
CIMGUI_API bool igImplDX9_CreateDeviceObjects();
CIMGUI_API void igImplDX9_InvalidateDeviceObjects();

#endif // CIMGUI_IMPL_DX9_INCLUDED
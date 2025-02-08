#ifndef CIMGUI_IMPL_WIN32_INCLUDED
#define CIMGUI_IMPL_WIN32_INCLUDED

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

CIMGUI_API bool igImplWin32_Init(void* hwnd);
CIMGUI_API bool igImplWin32_InitForOpenGL(void* hwnd);
CIMGUI_API void igImplWin32_Shutdown();
CIMGUI_API void igImplWin32_NewFrame();

#if 0
CIMGUI_API LRESULT igImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

CIMGUI_API void igImplWin32_EnableDpiAwareness();
CIMGUI_API float igImplWin32_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
CIMGUI_API float igImplWin32_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor
CIMGUI_API void igImplWin32_EnableAlphaCompositing(void* hwnd);   // HWND hwnd

#endif // CIMGUI_IMPL_WIN32_INCLUDED
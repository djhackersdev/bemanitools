#include <windows.h>

#include <stdint.h>

#include "ddrhook-util/gfx.h"

#include "hook/pe.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

static LONG(STDCALL *real_ChangeDisplaySettingsExA)(
    char *dev_name, DEVMODE *dev_mode, HWND hwnd, DWORD flags, void *param);
static LRESULT(STDCALL *real_SendMessageW)(
    HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static HWND(STDCALL *real_CreateWindowExW)(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);
static LONG(STDCALL *real_SetWindowLongW)(
    HWND hWnd, int nIndex, LONG dwNewLong);
static BOOL(STDCALL *real_SetWindowPos)(
    HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

static LONG STDCALL my_ChangeDisplaySettingsExA(
    char *dev_name, DEVMODE *dev_mode, HWND hwnd, DWORD flags, void *param);
static SHORT STDCALL my_GetKeyState(int vk);
static LRESULT STDCALL
my_SendMessageW(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static HWND STDCALL my_CreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);
static HWND STDCALL my_CreateWindowExW(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);
static LONG STDCALL my_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong);
static BOOL STDCALL my_SetWindowPos(
    HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

static const struct hook_symbol misc_user32_syms[] = {
    {
        .name = "ChangeDisplaySettingsExA",
        .patch = my_ChangeDisplaySettingsExA,
        .link = (void **) &real_ChangeDisplaySettingsExA,
    },
    {
        .name = "SendMessageW",
        .patch = my_SendMessageW,
        .link = (void **) &real_SendMessageW,
    },
    {
        .name = "GetKeyState",
        .patch = my_GetKeyState,
    },
    {
        .name = "CreateWindowExA",
        .patch = my_CreateWindowExA,
    },
    {
        .name = "CreateWindowExW",
        .patch = my_CreateWindowExW,
        .link = (void **) &real_CreateWindowExW,
    },
    {
        .name = "SetWindowLongW",
        .patch = my_SetWindowLongW,
        .link = (void **) &real_SetWindowLongW,
    },
    {
        .name = "SetWindowPos",
        .patch = my_SetWindowPos,
        .link = (void **) &real_SetWindowPos,
    },
};

static LONG STDCALL my_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
    if (nIndex == GWL_STYLE)
        dwNewLong |= WS_OVERLAPPEDWINDOW;
    return real_SetWindowLongW(hWnd, nIndex, dwNewLong);
}

static BOOL STDCALL my_SetWindowPos(
    HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    return true;
}

static LONG STDCALL my_ChangeDisplaySettingsExA(
    char *dev_name, DEVMODE *dev_mode, HWND hwnd, DWORD flags, void *param)
{
    if (gfx_get_windowed()) {
        return DISP_CHANGE_SUCCESSFUL;
    } else {
        return real_ChangeDisplaySettingsExA(
            dev_name, dev_mode, hwnd, flags, param);
    }
}

static void calc_win_size_with_framed(
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

static HWND STDCALL my_CreateWindowExW(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    if (gfx_get_windowed()) {
        /* use a different style */
        dwStyle |= WS_OVERLAPPEDWINDOW;
        /* also show mouse cursor */
        ShowCursor(TRUE);
    }

    if (!lpWindowName)
        lpWindowName = L"Dance Dance Revolution";

    HWND hwnd = real_CreateWindowExW(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,
        X,
        Y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam);

    if (hwnd == INVALID_HANDLE_VALUE) {
        return hwnd;
    }

    if (gfx_get_windowed()) {
        /* we have to adjust the window size, because the window needs to be a
           slightly bigger than the rendering resolution (window caption and
           stuff is included in the window size) */
        WINDOWPOS wp;
        calc_win_size_with_framed(hwnd, X, Y, nWidth, nHeight, &wp);
        SetWindowPos(hwnd, 0, wp.x, wp.y, wp.cx, wp.cy, 0);
        X = wp.x;
        Y = wp.y;
        nWidth = wp.cx;
        nHeight = wp.cy;
    }

    return hwnd;
}

static HWND STDCALL my_CreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    LPWSTR longWindowName = NULL;
    LPWSTR longClassName = NULL;
    if (lpWindowName)
        longWindowName = str_widen(lpWindowName);
    if (lpClassName)
        longClassName = str_widen(lpClassName);

    HWND ret = my_CreateWindowExW(
        dwExStyle,
        longClassName,
        longWindowName,
        dwStyle,
        X,
        Y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam);

    if (longWindowName)
        free(longWindowName);
    if (longClassName)
        free(longClassName);

    return ret;
}

static SHORT STDCALL my_GetKeyState(int vk)
{
    /* yeah we kinda have our own keyboard input thing */
    return 0;
}

static LRESULT STDCALL
my_SendMessageW(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (hwnd == HWND_BROADCAST) {
        /* OK COOL STORY BRO */
        log_misc("Dropping message sent to HWND_BROADCAST");

        return TRUE;
    } else {
        return real_SendMessageW(hwnd, msg, wparam, lparam);
    }
}

void misc_insert_hooks(HMODULE target)
{
    hook_table_apply(
        target, "user32.dll", misc_user32_syms, lengthof(misc_user32_syms));

    log_info("Inserted miscellaneous hooks into %p", target);
}

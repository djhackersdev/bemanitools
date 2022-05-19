#include <windows.h>

#include <stdint.h>

#include "ddrhook-util/gfx.h"

#include "hook/pe.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

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

static const struct hook_symbol misc_user32_syms[] = {
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
};

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
    // This breaks DDR X, X2 and X3
    if (gfx_get_is_modern()) {
        return real_CreateWindowExW(
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
    }

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
        gfx_d3d9_calc_win_size_with_framed(hwnd, X, Y, nWidth, nHeight, &wp);
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

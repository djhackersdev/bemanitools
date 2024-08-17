#define LOG_MODULE "jbhook1-gfx"

#include <windows.h>

#include <stdlib.h>

#include "hook/table.h"

#include "iface-core/log.h"

#include "jbhook1/config-gfx.h"

#include "jbhook-util-p3io/gfx.h"

#include "util/defs.h"

static jbhook1_config_gfx_t _jbhook1_gfx_config;

static HWND CDECL
my_mwindow_create(HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);
static HWND(CDECL *real_mwindow_create)(
    HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);

static BOOL STDCALL my_CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);
static BOOL(STDCALL *real_CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

static const struct hook_symbol mwindow_hook_syms[] = {
    {
        .name = "mwindow_create",
        .patch = my_mwindow_create,
        .link = (void **) &real_mwindow_create,
    },
};

static const struct hook_symbol kernel32_hook_syms[] = {
    {
        .name = "CreateProcessA",
        .patch = my_CreateProcessA,
        .link = (void **) &real_CreateProcessA,
    },
};

static HWND CDECL my_mwindow_create(
    HINSTANCE hinstance,
    void *callback,
    const char *window_title,
    DWORD window_width,
    DWORD window_height,
    BOOL fullscreen)
{
    DWORD tmp;

    fullscreen = !_jbhook1_gfx_config.windowed;

    if (_jbhook1_gfx_config.vertical) {
        tmp = window_width;
        window_width = window_height;
        window_height = tmp;
    }

    return real_mwindow_create(
        hinstance,
        callback,
        window_title,
        window_width,
        window_height,
        fullscreen);
}

static BOOL STDCALL my_CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    LPSTR rot_arg;

    // de-rotate the error window if needed. In theory this should parse the
    // commandline properly, in practice this works on all the .exe jubeats.
    if (_jbhook1_gfx_config.vertical && lpCommandLine) {
        if ((rot_arg = strstr(lpCommandLine, " -R90 "))) { // jubeat 01
            memcpy(rot_arg, " -R0  ", strlen(" -R0  "));
        } else if ((rot_arg =
                        strstr(lpCommandLine, " -rot:90 "))) { // jubeat 02
            memcpy(rot_arg, " -rot:0  ", strlen(" -rot:0  "));
        }
    }

    return real_CreateProcessA(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation);
}

void jbhook1_gfx_init(const jbhook1_config_gfx_t *config)
{
    memcpy(&_jbhook1_gfx_config, config, sizeof(_jbhook1_gfx_config));

    hook_table_apply(
            NULL, "mwindow.dll", mwindow_hook_syms, lengthof(mwindow_hook_syms));
    hook_table_apply(
            NULL,
            "kernel32.dll",
            kernel32_hook_syms,
            lengthof(kernel32_hook_syms));

    log_info("Inserted gfx hooks");

    if (_jbhook1_gfx_config.vertical) {
        jbhook_util_gfx_install_vertical_hooks();
    }
}

void jbhook1_gfx_fini()
{
    hook_table_revert(
            NULL, "mwindow.dll", mwindow_hook_syms, lengthof(mwindow_hook_syms));
    hook_table_revert(
            NULL,
            "kernel32.dll",
            kernel32_hook_syms,
            lengthof(kernel32_hook_syms));

    log_info("Removed gfx hooks");
}
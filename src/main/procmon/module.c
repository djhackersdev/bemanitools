#define LOG_MODULE "procmon-module"

#include <windows.h>

#include "core/log.h"

#include "hook/table.h"

#include "util/str.h"

static HMODULE (STDCALL *real_GetModuleHandleA)(LPCSTR lpModuleName);
static BOOL (STDCALL *real_GetModuleHandleExA)(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule);
static BOOL (STDCALL *real_GetModuleHandleExW)(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule);
static HMODULE (STDCALL *real_GetModuleHandleW)(LPCWSTR lpModuleName);
static HMODULE (STDCALL *real_LoadLibraryA)(LPCSTR lpLibFileName);
static HMODULE (STDCALL *real_LoadLibraryW)(LPCWSTR lpLibFileName);
static HMODULE (STDCALL *real_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD  dwFlags);
static HMODULE (STDCALL *real_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD  dwFlags);
static FARPROC (STDCALL *real_GetProcAddress)(HMODULE hModule, LPCSTR  lpProcName);

static HMODULE STDCALL my_GetModuleHandleA(LPCSTR lpModuleName);
static BOOL STDCALL my_GetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule);
static BOOL STDCALL my_GetModuleHandleExW(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule);
static HMODULE STDCALL my_GetModuleHandleW(LPCWSTR lpModuleName);
static HMODULE STDCALL my_LoadLibraryA(LPCSTR lpLibFileName);
static HMODULE STDCALL my_LoadLibraryW(LPCWSTR lpLibFileName);
static HMODULE STDCALL my_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD  dwFlags);
static HMODULE STDCALL my_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD  dwFlags);
static FARPROC STDCALL my_GetProcAddress(HMODULE hModule, LPCSTR  lpProcName);

static const struct hook_symbol _procmon_module_hook_syms[] = {
    {
        .name = "GetModuleHandleA",
        .patch = my_GetModuleHandleA,
        .link = (void **) &real_GetModuleHandleA,
    },
    {
        .name = "GetModuleHandleExA",
        .patch = my_GetModuleHandleExA,
        .link = (void **) &real_GetModuleHandleExA,
    },
    {
        .name = "GetModuleHandleExW",
        .patch = my_GetModuleHandleExW,
        .link = (void **) &real_GetModuleHandleExW,
    },
    {
        .name = "GetModuleHandleW",
        .patch = my_GetModuleHandleW,
        .link = (void **) &real_GetModuleHandleW,
    },
    {
        .name = "LoadLibraryA",
        .patch = my_LoadLibraryA,
        .link = (void **) &real_LoadLibraryA,
    },
    {
        .name = "LoadLibraryW",
        .patch = my_LoadLibraryW,
        .link = (void **) &real_LoadLibraryW,
    },
    {
        .name = "LoadLibraryExA",
        .patch = my_LoadLibraryExA,
        .link = (void **) &real_LoadLibraryExA,
    },
    {
        .name = "LoadLibraryExW",
        .patch = my_LoadLibraryExW,
        .link = (void **) &real_LoadLibraryExW,
    },
    {
        .name = "GetProcAddress",
        .patch = my_GetProcAddress,
        .link = (void **) &real_GetProcAddress,
    },
};

static HMODULE STDCALL my_GetModuleHandleA(LPCSTR lpModuleName)
{
    HMODULE result;

    log_misc("GetModuleHandleA(lpModuleName %s)", lpModuleName);

    result = real_GetModuleHandleA(lpModuleName);

    log_misc("GetModuleHandleA(lpModuleName %s) = %p", lpModuleName, result);

    return result;
}

static BOOL STDCALL my_GetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule)
{
    BOOL result;

    log_misc("GetModuleHandleExA(dwFlags %lu, lpModuleName %s, phModule %p)", dwFlags, lpModuleName, phModule);

    result = real_GetModuleHandleExA(dwFlags, lpModuleName, phModule);

    log_misc("GetModuleHandleExA(dwFlags %lu, lpModuleName %s, phModule %p) = %d", dwFlags, lpModuleName, phModule, result);

    return result;
}

static BOOL STDCALL my_GetModuleHandleExW(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule)
{
    BOOL result;
    char *tmp;

    wstr_narrow(lpModuleName, &tmp);

    log_misc("GetModuleHandleExW(dwFlags %lu, lpModuleName %s, phModule %p)", dwFlags, tmp, phModule);

    result = real_GetModuleHandleExW(dwFlags, lpModuleName, phModule);

    log_misc("GetModuleHandleExW(dwFlags %lu, lpModuleName %s, phModule %p) = %d", dwFlags, tmp, phModule, result);

    free(tmp);

    return result;
}

static HMODULE STDCALL my_GetModuleHandleW(LPCWSTR lpModuleName)
{
    HMODULE result;
    char *tmp;

    wstr_narrow(lpModuleName, &tmp);

    log_misc("GetModuleHandleW(lpModuleName %s)", tmp);

    result = real_GetModuleHandleW(lpModuleName);

    log_misc("GetModuleHandleW(lpModuleName %s) = %p", tmp, result);

    free(tmp);

    return result;
}

static HMODULE STDCALL my_LoadLibraryA(LPCSTR lpLibFileName)
{
    HMODULE result;

    log_misc("LoadLibraryA(lpLibFileName %s)", lpLibFileName);

    result = real_LoadLibraryA(lpLibFileName);

    log_misc("LoadLibraryA(lpLibFileName %s) = %p", lpLibFileName, result);

    return result;
}

static HMODULE STDCALL my_LoadLibraryW(LPCWSTR lpLibFileName)
{
    HMODULE result;
    char *tmp;

    wstr_narrow(lpLibFileName, &tmp);

    log_misc("LoadLibraryW(lpLibFileName %s)", tmp);

    result = real_LoadLibraryW(lpLibFileName);

    log_misc("LoadLibraryW(lpLibFileName %s) = %p", tmp, result);

    free(tmp);

    return result;
}

static HMODULE STDCALL my_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE result;

    log_misc("LoadLibraryExA(lpLibFileName %s, hFile %p, dwFlags %lu)", lpLibFileName, hFile, dwFlags);

    result = real_LoadLibraryExA(lpLibFileName, hFile, dwFlags);

    log_misc("LoadLibraryExA(lpLibFileName %s, hFile %p, dwFlags %lu) = %p", lpLibFileName, hFile, dwFlags, result);

    return result;
}

static HMODULE STDCALL my_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE result;
    char *tmp;

    wstr_narrow(lpLibFileName, &tmp);

    log_misc("LoadLibraryExA(lpLibFileName %s, hFile %p, dwFlags %lu)", tmp, hFile, dwFlags);

    result = real_LoadLibraryExW(lpLibFileName, hFile, dwFlags);

    log_misc("LoadLibraryExA(lpLibFileName %s, hFile %p, dwFlags %lu) = %p", tmp, hFile, dwFlags, result);

    free(tmp);

    return result;
}

static FARPROC STDCALL my_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    FARPROC result;

    log_misc("GetProcAddress(hModule %p, lpProcName %s)", hModule, lpProcName);

    result = real_GetProcAddress(hModule, lpProcName);

    log_misc("GetProcAddress(hModule %p, lpProcName %s = %p", hModule, lpProcName, result);

    return result;
}

void procmon_module_init()
{
    hook_table_apply(
        NULL, "kernel32.dll", _procmon_module_hook_syms, lengthof(_procmon_module_hook_syms));

    log_misc("init");
}

void procmon_module_fini()
{
    hook_table_revert(
        NULL, "kernel32.dll", _procmon_module_hook_syms, lengthof(_procmon_module_hook_syms));
    
    log_misc("fini");
}
#define LOG_MODULE "path"

#include <stdint.h>
#include <string.h>
#include <windows.h>

#include "core/log.h"

#include "hook/table.h"

#include "iidxhook5-cn/path.h"

#include "util/str.h"

#define PATH_A "D:/JDZ-001/contents/"
#define PATH_W L"D:/JDZ-001/contents/"

static HANDLE WINAPI my_CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static HANDLE(WINAPI *real_CreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);

static HANDLE WINAPI
my_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
static HANDLE(WINAPI *real_FindFirstFileA)(
    LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);

static const struct hook_symbol iidxhook5_cn_path_hook_syms[] = {
    {.name = "CreateFileW",
     .patch = my_CreateFileW,
     .link = (void **) &real_CreateFileW},
    {.name = "FindFirstFileA",
     .patch = my_FindFirstFileA,
     .link = (void **) &real_FindFirstFileA},
};

static HANDLE WINAPI my_CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    if (lpFileName != NULL && wcsstr(lpFileName, PATH_W) == lpFileName) {
        wchar_t relative[MAX_PATH] = L".";
        wstr_cat(relative, MAX_PATH, lpFileName + wcslen(PATH_W) - 1);
        return real_CreateFileW(
            relative,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
    }

    return real_CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

static HANDLE WINAPI
my_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    if (lpFileName != NULL && strstr(lpFileName, PATH_A) == lpFileName) {
        char relative[MAX_PATH] = ".";
        str_cat(relative, MAX_PATH, lpFileName + strlen(PATH_A) - 1);
        return real_FindFirstFileA(relative, lpFindFileData);
    }

    return real_FindFirstFileA(lpFileName, lpFindFileData);
}

void iidxhook5_cn_path_init()
{
    hook_table_apply(
        NULL,
        "kernel32.dll",
        iidxhook5_cn_path_hook_syms,
        lengthof(iidxhook5_cn_path_hook_syms));

    log_info("Inserted path hooks");
}

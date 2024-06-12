#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hook/table.h"

#include "iface-core/log.h"

#include "util/defs.h"
#include "util/mem.h"
#include "util/str.h"

static BOOL STDCALL my_SetCurrentDirectoryA(LPCSTR lpPathName);
static HANDLE STDCALL
my_FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
static HANDLE STDCALL my_CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static BOOL WINAPI my_CreateDirectoryW(
    LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static BOOL(STDCALL *real_SetCurrentDirectoryA)(LPCSTR lpPathName);
static HANDLE(STDCALL *real_FindFirstFileW)(
    LPCWSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
static HANDLE(STDCALL *real_CreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static BOOL(WINAPI *real_CreateDirectoryW)(
    LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static const struct hook_symbol filesystem_hook_syms[] = {
    {.name = "CreateFileW",
     .patch = my_CreateFileW,
     .link = (void **) &real_CreateFileW},
    {
        .name = "SetCurrentDirectoryA",
        .patch = my_SetCurrentDirectoryA,
        .link = (void **) &real_SetCurrentDirectoryA,
    },
    {.name = "FindFirstFileW",
     .patch = my_FindFirstFileW,
     .link = (void **) &real_FindFirstFileW},
    {.name = "CreateDirectoryW",
     .patch = my_CreateDirectoryW,
     .link = (void **) &real_CreateDirectoryW},
};

void ddrhook1_get_launcher_path_parts(
    char **output_path, char **output_foldername)
{
    char module_path[MAX_PATH];
    char launcher_path[MAX_PATH];

    memset(launcher_path, 0, MAX_PATH);

    if (output_path != NULL)
        *output_path = NULL;

    if (output_foldername != NULL)
        *output_foldername = NULL;

    if (GetModuleFileNameA(NULL, module_path, sizeof(module_path)) == 0)
        return;

    char *filename_ptr = NULL;
    if (GetFullPathNameA(
            module_path, sizeof(launcher_path), launcher_path, &filename_ptr) ==
        0)
        return;

    if (filename_ptr != NULL)
        launcher_path[strlen(launcher_path) - strlen(filename_ptr) - 1] = 0;

    // Trim tailing slashes
    for (int i = strlen(launcher_path) - 1; i > 0; i--) {
        if (launcher_path[i] != '\\' && launcher_path[i] != '/')
            break;

        launcher_path[i] = 0;
    }

    int idx_folder = 0;
    for (idx_folder = strlen(launcher_path); idx_folder - 1 > 0; idx_folder--) {
        if (launcher_path[idx_folder - 1] == '\\' ||
            launcher_path[idx_folder - 1] == '/') {
            break;
        }
    }

    if (output_foldername != NULL) {
        int len = strlen(launcher_path) - idx_folder;

        if (len < 0)
            len = 0;

        *output_foldername = (char *) xmalloc(len + 2);
        memset(*output_foldername, 0, len + 1);
        strncpy(*output_foldername, launcher_path + idx_folder, len);
    }

    if (output_path != NULL) {
        size_t len = idx_folder - 1 < 0 ? 0 : idx_folder - 1;

        if (len > strlen(launcher_path))
            len = strlen(launcher_path);

        *output_path = (char *) xmalloc(len + 1);
        memset(*output_path, 0, len + 1);
        strncpy(*output_path, launcher_path, len);
    }
}

static wchar_t *ddrhook1_filesystem_get_path(LPCWSTR path)
{
    wchar_t *new_path = NULL;

    // char *tmp;
    // wstr_narrow(path, &tmp);
    // log_misc("path: %s", tmp);
    // free(tmp);

    // Deal with hardcoded paths: D:/HDX, E:/conf, E:/conf/nvram, E:/conf/raw,
    // F:/update, ...
    if (wstr_insensitive_eq(path, L"D:/HDX") ||
        wstr_insensitive_eq(path, L"D:\\HDX") ||
        wstr_insensitive_eq(path, L"D:/JDX") ||
        wstr_insensitive_eq(path, L"D:\\JDX")) {
        char *_new_path;
        ddrhook1_get_launcher_path_parts(&_new_path, NULL);

        if (_new_path) {
            new_path = str_widen(_new_path);
            return new_path;
        }
    } else if (
        wcslen(path) >= 7 &&
        (wcsnicmp(path, L"E:/conf", 7) == 0 ||
         wcsnicmp(path, L"E:\\conf", 7) == 0)) {
        char *launcher_folder;
        wchar_t *sub_path;

        ddrhook1_get_launcher_path_parts(NULL, &launcher_folder);
        sub_path = wcsstr(path, L"conf");

        if (sub_path && launcher_folder) {
            wchar_t *launcher_folder_w = str_widen(launcher_folder);
            new_path = (wchar_t *) xmalloc(MAX_PATH * sizeof(wchar_t));
            swprintf(
                new_path, MAX_PATH, L"%s\\%s", launcher_folder_w, sub_path);
            free(launcher_folder_w);
            return new_path;
        }
    } else if (
        wstr_insensitive_eq(path, L"F:/update") ||
        wstr_insensitive_eq(path, L"F:\\update") ||
        wstr_insensitive_eq(path, L".\\F:/update") ||
        wstr_insensitive_eq(path, L".\\F:\\update")) {
        char *launcher_folder;
        wchar_t *sub_path;

        ddrhook1_get_launcher_path_parts(NULL, &launcher_folder);
        sub_path = wcsstr(path, L"update");

        if (sub_path && launcher_folder) {
            wchar_t *launcher_folder_w = str_widen(launcher_folder);
            new_path = (wchar_t *) xmalloc(MAX_PATH * sizeof(wchar_t));
            swprintf(
                new_path, MAX_PATH, L"%s\\%s", launcher_folder_w, sub_path);
            free(launcher_folder_w);
            return new_path;
        }
    } else if (
        wcslen(path) >= 24 &&
        (wcsnicmp(path, L"D:/JDX/JDX-001/contents/", 24) == 0 ||
         wcsnicmp(path, L"D:\\JDX\\JDX-001\\contents\\", 24) == 0)) {
        char *content_path;

        ddrhook1_get_launcher_path_parts(&content_path, NULL);

        if (content_path) {
            wchar_t *content_path_w = str_widen(content_path);
            new_path = (wchar_t *) xmalloc(MAX_PATH * sizeof(wchar_t));
            swprintf(new_path, MAX_PATH, L"%s\\%s", content_path_w, path + 24);
            free(content_path_w);
            return new_path;
        }
    } else if (
        wcslen(path) >= 7 &&
        (wcsnicmp(path, L"D:/HDX/", 7) == 0 ||
         wcsnicmp(path, L"D:\\HDX\\", 7) == 0 ||
         wcsnicmp(path, L"D:/JDX/", 7) == 0 ||
         wcsnicmp(path, L"D:\\JDX\\", 7) == 0)) {
        char *content_path;

        ddrhook1_get_launcher_path_parts(&content_path, NULL);

        if (content_path) {
            wchar_t *content_path_w = str_widen(content_path);
            new_path = (wchar_t *) xmalloc(MAX_PATH * sizeof(wchar_t));
            swprintf(new_path, MAX_PATH, L"%s\\%s", content_path_w, path + 7);
            free(content_path_w);
            return new_path;
        }
    }

    return NULL;
}

static BOOL STDCALL my_SetCurrentDirectoryA(LPCSTR lpPathName)
{
    wchar_t *_lpPathName = str_widen(lpPathName);
    wchar_t *_new_path = ddrhook1_filesystem_get_path(_lpPathName);
    char *new_path = NULL;

    if (_lpPathName != NULL) {
        free(_lpPathName);
    }

    if (_new_path != NULL) {
        wstr_narrow(_new_path, &new_path);
        free(_new_path);
    }

    if (new_path != NULL) {
        bool r = real_SetCurrentDirectoryA(new_path);
        log_misc(
            "SetCurrentDirectoryA remapped path %s -> %s",
            lpPathName,
            new_path);
        free(new_path);
        return r;
    }

    return real_SetCurrentDirectoryA(lpPathName);
}

static HANDLE STDCALL
my_FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    wchar_t *new_path = ddrhook1_filesystem_get_path(lpFileName);

    if (new_path) {
        HANDLE r = real_FindFirstFileW(new_path, lpFindFileData);

        char *tmp;
        wstr_narrow(new_path, &tmp);
        log_misc("FindFirstFileW remapped path: %s", tmp);
        free(tmp);

        free(new_path);
        return r;
    }

    return real_FindFirstFileW(lpFileName, lpFindFileData);
}

static HANDLE STDCALL my_CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    wchar_t *new_path = ddrhook1_filesystem_get_path(lpFileName);

    if (new_path) {
        HANDLE r = real_CreateFileW(
            new_path,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);

        // char *tmp;
        // wstr_narrow(new_path, &tmp);
        // log_misc("CreateFileW remapped path %s", tmp);
        // free(tmp);

        free(new_path);
        return r;
    }

    return real_CreateFileW(
        new_path ? new_path : lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

BOOL WINAPI my_CreateDirectoryW(
    LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    wchar_t *new_path = ddrhook1_filesystem_get_path(lpPathName);

    if (new_path) {
        BOOL r = real_CreateDirectoryW(new_path, lpSecurityAttributes);

        char *tmp;
        wstr_narrow(new_path, &tmp);
        log_misc("CreateDirectoryW remapped path %s", tmp);
        free(tmp);

        free(new_path);
        return r;
    }

    return real_CreateDirectoryW(lpPathName, lpSecurityAttributes);
}

void ddrhook1_filesystem_hook_init()
{
    hook_table_apply(
        NULL,
        "kernel32.dll",
        filesystem_hook_syms,
        lengthof(filesystem_hook_syms));

    log_info("Inserted filesystem hooks");
}
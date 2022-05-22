#include <windows.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

static BOOL STDCALL my_SetCurrentDirectoryA(LPCTSTR lpPathName);
static HANDLE STDCALL my_FindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData);
static HANDLE STDCALL my_CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static BOOL WINAPI my_CreateDirectoryA(
    LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static BOOL(STDCALL *real_SetCurrentDirectoryA)(LPCTSTR lpPathName);
static HANDLE(STDCALL *real_FindFirstFileA)(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData);
static HANDLE(STDCALL *real_CreateFileA)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static BOOL(WINAPI *real_CreateDirectoryA)(
    LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static const struct hook_symbol filesystem_hook_syms[] = {
    {
        .name = "CreateFileA",
        .patch = my_CreateFileA,
        .link = (void **) &real_CreateFileA
    },
    {
        .name = "SetCurrentDirectoryA",
        .patch = my_SetCurrentDirectoryA,
        .link = (void **) &real_SetCurrentDirectoryA,
    },
    {
        .name = "FindFirstFileA",
        .patch = my_FindFirstFileA,
        .link = (void **) &real_FindFirstFileA
    },
    {
        .name = "CreateDirectoryA",
        .patch = my_CreateDirectoryA,
        .link = (void **) &real_CreateDirectoryA
    },
};

void ddrhook1_get_launcher_path_parts(char **output_path, char **output_foldername) {
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
    if (GetFullPathNameA(module_path, sizeof(launcher_path), launcher_path, &filename_ptr) == 0)
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
        if (launcher_path[idx_folder - 1] == '\\' || launcher_path[idx_folder - 1] == '/') {
            break;
        }
    }

    if (output_foldername != NULL) {
        int len = strlen(launcher_path) - idx_folder;

        if (len < 0)
            len = 0;

        *output_foldername = (char*)xmalloc(len + 1);
        memset(*output_foldername, 0, len + 1);
        strncpy(*output_foldername, launcher_path + idx_folder, len);
    }

    if (output_path != NULL) {
        size_t len = idx_folder - 1 < 0 ? 0 : idx_folder - 1;

        if (len > strlen(launcher_path))
            len = strlen(launcher_path);

        *output_path = (char*)xmalloc(len + 1);
        memset(*output_path, 0, len + 1);
        strncpy(*output_path, launcher_path, len);
    }
}

static char *ddrhook1_filesystem_get_path(LPCTSTR path)
{
    char *new_path = NULL;

    // Hardcoded paths: D:/HDX, E:/conf, E:/conf/nvram, E:/conf/raw
    if (stricmp(path, "D:/HDX") == 0
    || stricmp(path, "D:\\HDX") == 0) {
        ddrhook1_get_launcher_path_parts(&new_path, NULL);
    } else if (strnicmp(path, "E:/conf", 7) == 0
    || strnicmp(path, "E:\\conf", 7) == 0) {
        char *launcher_folder;
        char *conf_path;

        ddrhook1_get_launcher_path_parts(NULL, &launcher_folder);
        conf_path = strstr(path, "conf");

        if (conf_path && launcher_folder) {
            new_path = (char*)xmalloc(MAX_PATH);
            sprintf(new_path, "%s\\%s", launcher_folder, conf_path);
        }
    }

    return new_path;
}

static BOOL STDCALL my_SetCurrentDirectoryA(LPCTSTR lpPathName)
{
    char *new_path = ddrhook1_filesystem_get_path(lpPathName);

    if (new_path != NULL) {
        bool r = real_SetCurrentDirectoryA(new_path);
        log_misc("SetCurrentDirectoryA remapped path %s -> %s", lpPathName, new_path);
        free(new_path);
        return r;
    }

    return real_SetCurrentDirectoryA(lpPathName);
}

static HANDLE STDCALL my_FindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData)
{
    char *new_path = ddrhook1_filesystem_get_path(lpFileName);

    if (new_path) {
        HANDLE r = real_FindFirstFileA(
            new_path,
            lpFindFileData);

        log_misc("FindFirstFileA remapped path: %s -> %s\n", lpFileName, new_path);

        free(new_path);
        return r;
    }

    return real_FindFirstFileA(
        lpFileName,
        lpFindFileData);
}

static HANDLE STDCALL my_CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    char *new_path = ddrhook1_filesystem_get_path(lpFileName);

    if (new_path) {
        HANDLE r = real_CreateFileA(
            new_path,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);

        log_misc("CreateFileA remapped path %s -> %s", lpFileName, new_path);

        free(new_path);
        return r;
    }

    return real_CreateFileA(
        new_path ? new_path : lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

BOOL WINAPI my_CreateDirectoryA(
    LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    char *new_path = ddrhook1_filesystem_get_path(lpPathName);

    if (new_path) {
        BOOL r = real_CreateDirectoryA(new_path, lpSecurityAttributes);

        log_misc("CreateDirectoryA remapped path %s -> %s", lpPathName, new_path);

        free(new_path);
        return r;
    }

    return real_CreateDirectoryA(lpPathName, lpSecurityAttributes);
}

void ddrhook1_filesystem_hook_init()
{
    hook_table_apply(
        NULL, "kernel32.dll", filesystem_hook_syms, lengthof(filesystem_hook_syms));

    log_info("Inserted filesystem hooks");
}
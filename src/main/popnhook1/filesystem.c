#define LOG_MODULE "filesystem-hook"

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "core/log.h"

#include "hook/table.h"

#include "util/defs.h"
#include "util/mem.h"
#include "util/str.h"

/* ------------------------------------------------------------------------- */

static HANDLE STDCALL my_CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static HANDLE STDCALL
my_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);

static HANDLE(STDCALL *real_CreateFileA)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static HANDLE(STDCALL *real_FindFirstFileA)(
    LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);

/* ------------------------------------------------------------------------- */

static const struct hook_symbol settings_hook_syms[] = {
    {.name = "CreateFileA",
     .patch = my_CreateFileA,
     .link = (void **) &real_CreateFileA},
    {.name = "FindFirstFileA",
     .patch = my_FindFirstFileA,
     .link = (void **) &real_FindFirstFileA},
};

/* ------------------------------------------------------------------------- */

static char *popnhook1_filesystem_get_path(LPCSTR lpFileName)
{
    if (lpFileName != NULL &&
        (tolower(lpFileName[0]) == 'd' || tolower(lpFileName[0]) == 'e') &&
        lpFileName[1] == ':') {
        char *new_path = (char *) xmalloc(MAX_PATH);

        char *data_ptr = strstr(lpFileName, "\\data\\");
        if (data_ptr == NULL)
            data_ptr = strstr(lpFileName, "\\prog\\");

        if (data_ptr != NULL) {
            // Fix a data path string
            strcpy(new_path, "..\\..");
            strcat(new_path, data_ptr);
        } else {
            strcpy(new_path, lpFileName);
            new_path[1] = '\\';
        }

        return new_path;
    }

    return NULL;
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
    char *new_path = popnhook1_filesystem_get_path(lpFileName);

    if (new_path != NULL) {
        // log_misc("Remapped settings path %s -> %s", lpFileName, new_path);

        return real_CreateFileA(
            new_path,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
    }

    HANDLE status = real_CreateFileA(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);

    if (status == NULL) {
        log_warning("Could not open %s\n", lpFileName);
    }

    return status;
}

static HANDLE STDCALL
my_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    // pop'n 15 makes use of FindFirstFileA
    char *new_path = popnhook1_filesystem_get_path(lpFileName);

    if (new_path != NULL) {
        // log_misc("Remapped settings path 2 %s -> %s", lpFileName, new_path);

        return real_FindFirstFileA(new_path, lpFindFileData);
    }

    return real_FindFirstFileA(lpFileName, lpFindFileData);
}

/* ------------------------------------------------------------------------- */

void filesystem_init(void)
{
    hook_table_apply(
        NULL, "kernel32.dll", settings_hook_syms, lengthof(settings_hook_syms));

    log_info("Inserted filesystem hooks");
}

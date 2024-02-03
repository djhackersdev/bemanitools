#define LOG_MODULE "procmon-file"

#include <windows.h>

#include "hook/table.h"

#include "util/log.h"
#include "util/str.h"

static HANDLE (STDCALL *real_CreateFileW)(
    const wchar_t *lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    SECURITY_ATTRIBUTES *lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes,
    HANDLE hTemplateFile);
static HANDLE (STDCALL *real_CreateFileA)(
    const char *lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    SECURITY_ATTRIBUTES *lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes,
    HANDLE hTemplateFile);

static HANDLE STDCALL my_CreateFileW(
    const wchar_t *lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    SECURITY_ATTRIBUTES *lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes,
    HANDLE hTemplateFile);

static HANDLE STDCALL my_CreateFileA(
    const char *lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    SECURITY_ATTRIBUTES *lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes,
    HANDLE hTemplateFile);

static const struct hook_symbol _procmon_file_hook_syms[] = {
    {
        .name = "CreateFileW",
        .patch = my_CreateFileW,
        .link = (void **) &real_CreateFileW,
    },
    {
        .name = "CreateFileA",
        .patch = my_CreateFileA,
        .link = (void **) &real_CreateFileA,
    },
};

static HANDLE STDCALL my_CreateFileW(
    const wchar_t *lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    SECURITY_ATTRIBUTES *lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    HANDLE result;
    char *tmp;

    wstr_narrow(lpFileName, &tmp);

    log_misc("CreateFileW(lpFileName %s, dwDesiredAccess 0x%X, dwShareMode 0x%X, dwCreationDisposition 0x%X, dwFlagsAndAttributes 0x%X, hTemplateFile %p)",
        tmp, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    result = real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    log_misc("CreateFileW(lpFileName %s, dwDesiredAccess 0x%X, dwShareMode 0x%X, dwCreationDisposition 0x%X, dwFlagsAndAttributes 0x%X, hTemplateFile %p) = %p",
        tmp, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, result);

    free(tmp);

    return result;
}

static HANDLE STDCALL my_CreateFileA(
    const char *lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    SECURITY_ATTRIBUTES *lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    HANDLE result;

    log_misc("CreateFileA(lpFileName %s, dwDesiredAccess 0x%X, dwShareMode 0x%X, dwCreationDisposition 0x%X, dwFlagsAndAttributes 0x%X, hTemplateFile %p)",
        lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    result = real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    log_misc("CreateFileA(lpFileName %s, dwDesiredAccess 0x%X, dwShareMode 0x%X, dwCreationDisposition 0x%X, dwFlagsAndAttributes 0x%X, hTemplateFile %p) = %p",
        lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, result);

    return result;
}

void procmon_file_init()
{
    hook_table_apply(
        NULL, "kernel32.dll", _procmon_file_hook_syms, lengthof(_procmon_file_hook_syms));

    log_misc("init");
}

void procmon_file_fini()
{
    hook_table_revert(NULL, "kernel32.dll", _procmon_file_hook_syms, lengthof(_procmon_file_hook_syms));

    log_misc("fini");
}
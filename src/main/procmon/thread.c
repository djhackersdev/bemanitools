#define LOG_MODULE "procmon-thread"

#include <windows.h>

#include "hook/table.h"

#include "util/log.h"

#ifdef _WIN64
    #define SIZE_T_FORMAT_SPECIFIER "llu"
#else
    #define SIZE_T_FORMAT_SPECIFIER "lu"
#endif

static HANDLE (STDCALL *real_CreateThread)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, 
LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);

static HANDLE STDCALL my_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, 
LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);

static const struct hook_symbol _procmon_thread_hook_syms[] = {
    {
        .name = "CreateThread",
        .patch = my_CreateThread,
        .link = (void **) &real_CreateThread,
    },
};

static HANDLE STDCALL my_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
    HANDLE result;

    log_misc("CreateThread(lpThreadAttributes %p, dwStackSize %" SIZE_T_FORMAT_SPECIFIER ", lpStartAddress %p, lpParameter %p, dwCreationFlags %lu, lpThreadId %p)",
        lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

    result = real_CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

    log_misc("CreateThread(lpThreadAttributes %p, dwStackSize %" SIZE_T_FORMAT_SPECIFIER ", lpStartAddress %p, lpParameter %p, dwCreationFlags %lu, lpThreadId %p) = %p, tid %lu",
        lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId, result, lpThreadId ? *lpThreadId : -1);

    return result;
}

void procmon_thread_init()
{
    hook_table_apply(
        NULL, "kernel32.dll", _procmon_thread_hook_syms, lengthof(_procmon_thread_hook_syms));

    log_misc("init");
}

void procmon_thread_fini()
{
    hook_table_revert(
        NULL, "kernel32.dll", _procmon_thread_hook_syms, lengthof(_procmon_thread_hook_syms));

    log_misc("fini");
}
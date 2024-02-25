#define LOG_MODULE "util-proc"

#include <windows.h>
#include <winnt.h>

#include <stdbool.h>
#include <stdint.h>

#include "core/log.h"

bool proc_is_running_as_admin_user()
{
    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    PSID sid;
    BOOL res;
    BOOL is_admin;

    res = AllocateAndInitializeSid(
        &authority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0,
        0,
        0,
        0,
        0,
        0,
        &sid);

    if (!res) {
        log_warning(
            "Failed to allocate memory for is admin check: %lX",
            GetLastError());
        return false;
    }

    is_admin = false;

    res = CheckTokenMembership(NULL, sid, &is_admin);

    if (!res) {
        log_warning(
            "Failed to check admin group membership: %lX", GetLastError());
        return false;
    }

    FreeSid(sid);

    return is_admin;
}

void proc_terminate_current_process(uint32_t exit_code)
{
    HANDLE hnd;

    hnd = OpenProcess(
        SYNCHRONIZE | PROCESS_TERMINATE, TRUE, GetCurrentProcessId());

    TerminateProcess(hnd, exit_code);
}
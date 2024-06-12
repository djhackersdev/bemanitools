#define LOG_MODULE "util-proc"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <psapi.h>
#include <winnt.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>

#include "iface-core/log.h"

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

bool proc_is_module_loaded(const char *name)
{
    HMODULE modules[1024];
    HANDLE process;
    DWORD size;
    char module_name[MAX_PATH];

    process = GetCurrentProcess();

    if (EnumProcessModules(process, modules, sizeof(modules), &size)) {
        for (int i = 0; i < (size / sizeof(HMODULE)); i++) {
            if (GetModuleFileNameEx(
                    process,
                    modules[i],
                    module_name,
                    sizeof(module_name) / sizeof(char))) {
                const char *p = strrchr(module_name, '\\');

                if (p != NULL) {
                    p++;
                } else {
                    p = module_name;
                }

                if (_stricmp(p, name) == 0) {
                    return true;
                }
            }
        }
    }

    return false;
}
#define LOG_MODULE "unis-version"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <shellapi.h>
// clang-format on

#include <stdio.h>

#include "core/log.h"

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/str.h"
#include "util/time.h"

LPWSTR *(*real_CommandLineToArgvW)(LPCWSTR lpCmdLine, int *pNumArgs);

LPWSTR *my_CommandLineToArgvW(LPCWSTR lpCmdLine, int *pNumArgs);

static const struct hook_symbol unis_shell32_syms[] = {
    {.name = "CommandLineToArgvW",
     .patch = my_CommandLineToArgvW,
     .link = (void **) &real_CommandLineToArgvW},
};

bool (*real_IsInComm)(void);
bool my_IsInComm(void);

bool (*real_IsConnectServer)(void);
bool my_IsConnectServer(void);

bool (*real_IsPlayerForbidState)(int pnum);
bool my_IsPlayerForbidState(int pnum);

static wchar_t unis_cmd_path[256 + 16];

static const struct hook_symbol unis_unisintr_syms[] = {
    {.name = "IsInComm",
     .patch = my_IsInComm,
     .link = (void **) &real_IsInComm},
    {.name = "IsConnectServer",
     .patch = my_IsConnectServer,
     .link = (void **) &real_IsConnectServer},
    {.name = "IsPlayerForbidState",
     .patch = my_IsPlayerForbidState,
     .link = (void **) &real_IsPlayerForbidState},
};

LPWSTR *my_CommandLineToArgvW(LPCWSTR lpCmdLine, int *pNumArgs)
{
    return real_CommandLineToArgvW(unis_cmd_path, pNumArgs);
}

bool my_IsInComm(void)
{
    return true;
}

bool my_IsConnectServer(void)
{
    return true;
}

bool my_IsPlayerForbidState(int pnum)
{
    return false;
}

void unis_version_hook_init(const char *unis_path)
{
    hook_table_apply(
        NULL, "Shell32.dll", unis_shell32_syms, lengthof(unis_shell32_syms));
    hook_table_apply(
        NULL, "unisintr.dll", unis_unisintr_syms, lengthof(unis_unisintr_syms));

    wcscpy(unis_cmd_path, L"launcher.exe ");
    wchar_t *end = wcsrchr(unis_cmd_path, L'\0');
    mbstowcs(end, unis_path, 256);

    log_info("Inserted hooks for unis version detection");
}
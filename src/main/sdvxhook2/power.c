#define LOG_MODULE "power-hook"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <initguid.h>
// clang-format on

#include <powrprof.h>

#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "iface-core/log.h"

#include "sdvxhook2/power.h"

static DWORD
my_PowerSetActiveScheme(HKEY UserRootPowerKey, const GUID *SchemeGuid);
static DWORD my_PowerWriteACValueIndex(
    HKEY RootPowerKey,
    const GUID *SchemeGuid,
    const GUID *SubGroupOfPowerSettingsGuid,
    const GUID *PowerSettingGuid,
    DWORD AcValueIndex);
static DWORD
my_PowerGetActiveScheme(HKEY UserRootPowerKey, GUID **ActivePolicyGuid);

static DWORD (*real_PowerSetActiveScheme)(
    HKEY UserRootPowerKey, const GUID *SchemeGuid);
static DWORD (*real_PowerWriteACValueIndex)(
    HKEY RootPowerKey,
    const GUID *SchemeGuid,
    const GUID *SubGroupOfPowerSettingsGuid,
    const GUID *PowerSettingGuid,
    DWORD AcValueIndex);
static DWORD (*real_PowerGetActiveScheme)(
    HKEY UserRootPowerKey, GUID **ActivePolicyGuid);

static const struct hook_symbol powerhook_pp_syms[] = {
    {.name = "PowerGetActiveScheme",
     .patch = my_PowerGetActiveScheme,
     .link = (void **) &real_PowerGetActiveScheme},
    {.name = "PowerWriteACValueIndex",
     .patch = my_PowerWriteACValueIndex,
     .link = (void **) &real_PowerWriteACValueIndex},
    {.name = "PowerSetActiveScheme",
     .patch = my_PowerSetActiveScheme,
     .link = (void **) &real_PowerSetActiveScheme},
};

static DWORD
my_PowerSetActiveScheme(HKEY UserRootPowerKey, const GUID *SchemeGuid)
{
    // stubbed
    return 0;
}
static DWORD my_PowerWriteACValueIndex(
    HKEY RootPowerKey,
    const GUID *SchemeGuid,
    const GUID *SubGroupOfPowerSettingsGuid,
    const GUID *PowerSettingGuid,
    DWORD AcValueIndex)
{
    // stubbed
    return 0;
}
static DWORD
my_PowerGetActiveScheme(HKEY UserRootPowerKey, GUID **ActivePolicyGuid)
{
    // stubbed
    return 0;
}

void powerhook_init()
{
    hook_table_apply(
        NULL, "PowrProf.dll", powerhook_pp_syms, lengthof(powerhook_pp_syms));
}

void powerhook_fini(void)
{
    return;
}
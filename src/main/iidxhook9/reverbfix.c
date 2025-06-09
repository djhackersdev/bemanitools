#define LOG_MODULE "reverbfix-hook"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <initguid.h>
// clang-format on

#include <combaseapi.h>

#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "iidxhook9/reverbfix.h"

#include "util/log.h"

DEFINE_GUID(GUID_DSFX_STANDARD_I3DL2REVERB, 0xEF985E71,0xD5C7,0x42D4,0xBA,0x4D,0x2D,0x07,0x3E,0x2E,0x96,0xF4);
DEFINE_GUID(GUID_DSFX_WAVES_REVERB,         0x87FC0268,0x9A55,0x4360,0x95,0xAA,0x00,0x4A,0x1D,0x9D,0xE2,0x6C);

static HRESULT my_CoCreateInstance(
    REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

static HRESULT (*real_CoCreateInstance)(
    REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

static const struct hook_symbol reverbfix_ole_syms[] = {
    {.name = "CoCreateInstance",
     .patch = my_CoCreateInstance,
     .link = (void **) &real_CoCreateInstance},
};

static HRESULT my_CoCreateInstance(
    REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    HRESULT result = real_CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    if (result == REGDB_E_CLASSNOTREG) {
        if (IsEqualGUID(rclsid, &GUID_DSFX_STANDARD_I3DL2REVERB)) {
            // replace effect used by REVERB EX with REVERB, based on CDmoSoundFx classes
            result = real_CoCreateInstance(&GUID_DSFX_WAVES_REVERB, pUnkOuter, dwClsContext, riid, ppv);

            log_info("DSFX_STANDARD_I3DL2REVERB class missing, replaced with DSFX_WAVES_REVERB");
        }
    }

    return result;
}

void reverbfixhook_init()
{
    hook_table_apply(
        NULL, "Ole32.dll", reverbfix_ole_syms, lengthof(reverbfix_ole_syms));
    log_info("Inserted reverbfix hook");
}

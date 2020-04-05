#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "dinputhook/device_dinput8.h"
#include "dinputhook/dinput.h"

#include <stdbool.h>

#include "hook/com-proxy.h"
#include "hook/pe.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/log.h"

static HRESULT STDCALL my_DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID *ppvOut,
    LPVOID punkOuter);

static HRESULT(STDCALL *real_DirectInput8Create)(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID *ppvOut,
    LPVOID punkOuter);

static const struct hook_symbol dinput_syms[] = {
    {
        .name = "DirectInput8Create",
        .patch = my_DirectInput8Create,
        .link = (void **) &real_DirectInput8Create,
    },
};

static HRESULT STDCALL my_CreateDevice(
    IDirectInput8W *self,
    REFGUID rguid,
    LPDIRECTINPUTDEVICE8W *lplpDirectInputDevice,
    LPUNKNOWN pUnkOuter)
{
    log_misc("IDirectInput8::CreateDevice hook hit");
    if (lplpDirectInputDevice == NULL) {
        return DIERR_INVALIDPARAM;
    }

    if (IsEqualGUID(rguid, &GUID_SysKeyboard) ||
        IsEqualGUID(rguid, &GUID_SysMouse)) {
        log_misc("Returning stub device");

        *lplpDirectInputDevice = di8_stub_device;
        di8_stub_device->lpVtbl->AddRef(di8_stub_device);

        return DI_OK;
    }

    return DIERR_NOINTERFACE;
}

static HRESULT STDCALL my_EnumDevices(
    IDirectInput8W *self,
    DWORD dwDevType,
    LPDIENUMDEVICESCALLBACKW lpCallback,
    LPVOID pvRef,
    DWORD dwFlags)
{
    log_misc("IDirectInput8::EnumDevices hook hit");

    return DI_OK;
}

static HRESULT STDCALL my_DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID *ppvOut,
    LPVOID punkOuter)
{
    IDirectInput8W *api;
    IDirectInput8WVtbl *api_vtbl;
    struct com_proxy *api_proxy;
    HRESULT res;

    log_info("DirectInput8Create hook hit");

    res = real_DirectInput8Create(
        hinst, dwVersion, riidltf, (LPVOID *) &api, punkOuter);
    if (res != DI_OK) {
        return res;
    }
    api_proxy = com_proxy_wrap(api, sizeof(*api->lpVtbl));
    api_vtbl = api_proxy->vptr;

    api_vtbl->EnumDevices = my_EnumDevices;
    api_vtbl->CreateDevice = my_CreateDevice;

    *(IDirectInput8W **) ppvOut = (IDirectInput8W *) api_proxy;

    return res;
}

void dinput_init(void)
{
    hook_table_apply(NULL, "dinput8.dll", dinput_syms, lengthof(dinput_syms));

    log_info("Inserted dinput hooks");
}

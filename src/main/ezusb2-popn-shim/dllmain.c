#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

#include <setupapi.h>

#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"

#include "hook/pe.h"

#include "hooklib/setupapi.h"

#include "ezusb2-popn-shim/proxy.h"

#include "util/log.h"

#define EZUSB_REAL_DLL_FILENAME "ezusb.dll"

static DWORD(STDCALL *real_entrypoint)(HMODULE self, DWORD reason, void *ctx);

static HMODULE real_pe = NULL;

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH && real_pe == NULL) {
        pe_hijack_entrypoint(EZUSB_REAL_DLL_FILENAME, &real_entrypoint);

        real_pe = GetModuleHandleA(EZUSB_REAL_DLL_FILENAME);

        ezusb2_proxy_initialize(real_pe);
        hook_setupapi_init(&ezusb2_emu_desc_device.setupapi);
    }

    if (real_pe != NULL)
        return real_entrypoint(real_pe, reason, ctx);

    return true;
}
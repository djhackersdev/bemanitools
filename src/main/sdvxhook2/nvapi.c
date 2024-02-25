#define LOG_MODULE "nvapi-hook"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <initguid.h>
// clang-format on

#include <powrprof.h>

#include <stdio.h>

#include "core/log.h"

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "sdvxhook2/nvapi.h"

static void *my_GetProcAddress(HMODULE dll, const char *name);

static void *(*real_GetProcAddress)(HMODULE dll, const char *name);

// nvapi64.dll
static void *my_nvapi_QueryInterface(uint32_t FunctionOffset);
static void *(*real_nvapi_QueryInterface)(uint32_t FunctionOffset);

static struct hook_symbol nvapihook_kernel_syms[] = {
    {.name = "GetProcAddress",
     .patch = my_GetProcAddress,
     .link = (void *) &real_GetProcAddress},
};

static void *my_GetProcAddress(HMODULE dll, const char *name)
{
    if (name != NULL && ((intptr_t) name) > UINT16_MAX) {
        // only strcmp is non-ordinal
        // https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress
        if (strcmp("nvapi_QueryInterface", name) == 0) {
            log_info("Request for stub %s", name);
            real_nvapi_QueryInterface = real_GetProcAddress(dll, name);
            return &my_nvapi_QueryInterface;
        }
    }

    // log_warning("Request for unknown stub %s", name);
    return real_GetProcAddress(dll, name);
}

#define OFFSET_NvAPI_DISP_SetDisplayConfig 0x5D8CF8DE

static void *my_nvapi_QueryInterface(uint32_t FunctionOffset)
{
    if (FunctionOffset == OFFSET_NvAPI_DISP_SetDisplayConfig) {
        log_info("Request for interface %x intercepted", FunctionOffset);
        // no point stubbing this, the error checking for now doesn't check the
        // response anyways
        return NULL;
    }

    log_info("Request for interface %x passed", FunctionOffset);
    return real_nvapi_QueryInterface(FunctionOffset);
}

void nvapihook_init()
{
    hook_table_apply(
        NULL,
        "kernel32.dll",
        nvapihook_kernel_syms,
        lengthof(nvapihook_kernel_syms));
}

void nvapihook_fini(void)
{
    return;
}
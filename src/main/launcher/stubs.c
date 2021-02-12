#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "hook/table.h"

#include "launcher/stubs.h"

#include "util/defs.h"
#include "util/log.h"

struct ikey_status {
    uint32_t field_0;
    uint8_t field_4;
    uint8_t field_5;
    uint8_t field_6;
    uint8_t field_7;
    uint32_t cert_valid_start;
    uint32_t cert_valid_end;
};

struct stub {
    const char *name;
    void *proc;
};

typedef void (*bt_get_fucked_t)(uint32_t /* even in 64-bit */ context);

static HMODULE STDCALL my_GetModuleHandleA(const char *name);
static HMODULE STDCALL my_GetModuleHandleW(const wchar_t *name);
static void *STDCALL my_GetProcAddress(HMODULE dll, const char *name);

static void bt_get_ikey_status(struct ikey_status *ik);

#if AVS_VERSION >= 1509
static void bt_get_fucked(bt_get_fucked_t callback, uint32_t ctx);
#endif

static void bt_fcheck_init(void);
static void *bt_fcheck_main(void *unknown);
static void bt_fcheck_finish(void);

static HMODULE(STDCALL *real_GetModuleHandleA)(const char *name);
static HMODULE(STDCALL *real_GetModuleHandleW)(const wchar_t *name);
static void *(STDCALL *real_GetProcAddress)(HMODULE dll, const char *name);

static struct stub stub_list[] = {
#if AVS_VERSION >= 1509
    {"k_bt0001", bt_get_ikey_status},
    {"k_bt0002", bt_get_fucked},
#else
    {"bt_get_ikey_status", bt_get_ikey_status},
#endif
    {"bt_fcheck_init", bt_fcheck_init},
    {"bt_fcheck_main", bt_fcheck_main},
    {"bt_fcheck_finish", bt_fcheck_finish},
};

static struct hook_symbol stub_hook_syms[] = {
    {.name = "GetModuleHandleA",
     .patch = my_GetModuleHandleA,
     .link = (void *) &real_GetModuleHandleA},
    {.name = "GetModuleHandleW",
     .patch = my_GetModuleHandleW,
     .link = (void *) &real_GetModuleHandleW},
    {.name = "GetProcAddress",
     .patch = my_GetProcAddress,
     .link = (void *) &real_GetProcAddress},
};

static HMODULE STDCALL my_GetModuleHandleA(const char *name)
{
    /* Check name for null because winio32 calls this with null parameters
       when the module is loaded on win xp (doesn't happen on win 7) */
    if (name != NULL &&
        (_stricmp(name, "kbt.dll") == 0 || _stricmp(name, "kld.dll") == 0)) {
        name = NULL;
    }

    return real_GetModuleHandleA(name);
}

static HMODULE STDCALL my_GetModuleHandleW(const wchar_t *name)
{
    /* Check name for null because winio32 calls this with null parameters
       when the module is loaded on win xp (doesn't happen on win 7) */
    if (name != NULL &&
        (wcsicmp(name, L"kbt.dll") == 0 || wcsicmp(name, L"kld.dll") == 0)) {
        name = NULL;
    }

    return real_GetModuleHandleW(name);
}

static void *STDCALL my_GetProcAddress(HMODULE dll, const char *name)
{
    size_t i;

    if (dll != real_GetModuleHandleA(NULL)) {
        return real_GetProcAddress(dll, name);
    }

    for (i = 0; i < lengthof(stub_list); i++) {
        if (strcmp(stub_list[i].name, name) == 0) {
            return stub_list[i].proc;
        }
    }

    log_warning("Request for unknown stub %s", name);

    return NULL;
}

void stubs_init(void)
{
    hook_table_apply(
        NULL, "kernel32.dll", stub_hook_syms, lengthof(stub_hook_syms));
}

static void bt_get_ikey_status(struct ikey_status *ik)
{
    memset(ik, 0, sizeof(*ik) * 2);

    ik[0].field_4 = true;
    ik[0].field_6 = true;
    ik[1].field_4 = true;
    ik[1].field_6 = true;

    /* Not strictly necessary */
    ik[0].cert_valid_end = -1;
    ik[1].cert_valid_end = -1;
}

#if AVS_VERSION >= 1509
static void bt_get_fucked(bt_get_fucked_t callback, uint32_t ctx)
{
    log_info(">>> k_bt0002");
    callback(ctx);
    log_info("<<< k_bt0002");
}
#endif

static void bt_fcheck_init(void)
{
}

static void *bt_fcheck_main(void *unknown)
{
    return 0;
}

static void bt_fcheck_finish(void)
{
}

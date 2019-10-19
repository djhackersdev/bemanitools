#include <windows.h>

#include "hook/table.h"

#include "hooklib/app.h"

#include "imports/avs.h"
#include "imports/eapki.h"

#include "util/log.h"
#include "util/str.h"

static dll_entry_init_t hook_dll_entry_init;
static dll_entry_main_t hook_dll_entry_main;
static dll_entry_init_t next_dll_entry_init;
static dll_entry_main_t next_dll_entry_main;
static void *(STDCALL *next_GetProcAddress)(HMODULE mod, const char *sym);

static void *STDCALL my_GetProcAddress(HMODULE mod, const char *sym);

static const struct hook_symbol mod_hooks[] = {
    {
        .name = "GetProcAddress",
        .patch = my_GetProcAddress,
        .link = (void **) &next_GetProcAddress,
    },
};

static void *STDCALL my_GetProcAddress(HMODULE mod, const char *sym)
{
    void *next;

    next = next_GetProcAddress(mod, sym);

    if (next == NULL || ((intptr_t) sym) <= UINT16_MAX) {
        return next;
    }

    if (str_eq(sym, "dll_entry_init")) {
        next_dll_entry_init = next;

        if (hook_dll_entry_init != NULL) {
            return hook_dll_entry_init;
        } else {
            return next;
        }
    } else if (str_eq(sym, "dll_entry_main")) {
        next_dll_entry_main = next;

        if (hook_dll_entry_main != NULL) {
            return hook_dll_entry_main;
        } else {
            return next;
        }
    } else {
        return next;
    }
}

void app_hook_init(dll_entry_init_t init, dll_entry_main_t main_)
{
    hook_dll_entry_init = init;
    hook_dll_entry_main = main_;
    hook_table_apply(NULL, "kernel32.dll", mod_hooks, lengthof(mod_hooks));
}

bool app_hook_invoke_init(char *sidcode, struct property_node *config)
{
    log_assert(sidcode != NULL);
    log_assert(config != NULL);

    return next_dll_entry_init(sidcode, config);
}

bool app_hook_invoke_main(void)
{
    return next_dll_entry_main();
}

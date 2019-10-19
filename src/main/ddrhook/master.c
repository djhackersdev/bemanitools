#include "ddrhook/master.h"
#include "ddrhook/dinput.h"
#include "ddrhook/gfx.h"
#include "ddrhook/misc.h"
#include "ddrhook/monitor.h"

#include "hook/table.h"

#include "p3ioemu/devmgr.h"

#include "util/defs.h"
#include "util/log.h"

static HMODULE(STDCALL *real_LoadLibraryA)(const char *name);

static HMODULE STDCALL my_LoadLibraryA(const char *name);

static const struct hook_symbol master_kernel32_syms[] = {
    {
        .name = "LoadLibraryA",
        .patch = my_LoadLibraryA,
        .link = (void **) &real_LoadLibraryA,
    },
};

static HMODULE STDCALL my_LoadLibraryA(const char *name)
{
    HMODULE result;

    result = GetModuleHandleA(name);

    if (result != NULL) {
        log_misc("LoadLibraryA(%s) -> %p [already loaded]", name, result);

        return result;
    }

    result = real_LoadLibraryA(name);
    log_misc("LoadLibraryA(%s) -> %p [newly loaded]", name, result);

    if (result == NULL) {
        return result;
    }

    master_insert_hooks(result);

    return result;
}

void master_insert_hooks(HMODULE target)
{
    /* Insert all other hooks here */

    p3io_setupapi_insert_hooks(target);
    monitor_setupapi_insert_hooks(target);
    misc_insert_hooks(target);
    dinput_init(target);
    gfx_insert_hooks(target);

    /* Insert dynamic loader hooks so that we can hook late-loaded modules */

    hook_table_apply(
        target,
        "kernel32.dll",
        master_kernel32_syms,
        lengthof(master_kernel32_syms));

    log_info("Inserted dynamic loader hooks into %p", target);
}

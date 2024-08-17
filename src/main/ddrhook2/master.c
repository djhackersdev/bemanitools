#include "ddrhook2/master.h"

#include "ddrhook-util/dinput.h"
#include "ddrhook-util/gfx.h"
#include "ddrhook-util/misc.h"
#include "ddrhook-util/monitor.h"

#include "hook/table.h"

#include "iface-core/log.h"

#include "p3ioemu/devmgr.h"

#include "util/defs.h"

static HMODULE(STDCALL *real_LoadLibraryA)(const char *name);

static HMODULE STDCALL my_LoadLibraryA(const char *name);

static const hook_d3d9_irp_handler_t ddrhook2_d3d9_handlers[] = {
    gfx_d3d9_irp_handler,
};

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

    ddrhook2_master_insert_hooks(result);

    return result;
}

void ddrhook2_master_insert_hooks(HMODULE target)
{
    /* Insert all other hooks here */

    p3io_setupapi_insert_hooks(target);
    monitor_setupapi_insert_hooks(target);
    misc_insert_hooks(target);
    dinput_init(target);
    gfx_insert_hooks(target);

    hook_d3d9_init(ddrhook2_d3d9_handlers, lengthof(ddrhook2_d3d9_handlers));

    /* Insert dynamic loader hooks so that we can hook late-loaded modules */

    hook_table_apply(
        target,
        "kernel32.dll",
        master_kernel32_syms,
        lengthof(master_kernel32_syms));

    log_info("Inserted dynamic loader hooks into %p", target);
}

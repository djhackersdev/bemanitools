#include "core/log.h"

#include "ddrhook1/master.h"

#include "ddrhook-util/dinput.h"
#include "ddrhook-util/gfx.h"
#include "ddrhook-util/monitor.h"

#include "hook/table.h"

#include "p3ioemu/devmgr.h"

#include "util/defs.h"

static HMODULE(STDCALL *real_LoadLibraryA)(const char *name);
static BOOL(STDCALL *real_IsDebuggerPresent)();

static HMODULE STDCALL my_LoadLibraryA(const char *name);
static BOOL STDCALL my_IsDebuggerPresent();

static const struct hook_symbol master_kernel32_syms[] = {
    {
        .name = "LoadLibraryA",
        .patch = my_LoadLibraryA,
        .link = (void **) &real_LoadLibraryA,
    },
    {
        .name = "IsDebuggerPresent",
        .patch = my_IsDebuggerPresent,
        .link = (void **) &real_IsDebuggerPresent,
    },
};

static BOOL STDCALL my_IsDebuggerPresent()
{
    // DDR X has different code paths for when a debugger is attached,
    // but we want the normal path and not the debugger path.
    // The biggest noticeable difference is that when a debugger is attached
    // it will show fway.mpg (a car driving) instead of the sky background
    // on the menus.
    return FALSE;
}

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

    ddrhook1_master_insert_hooks(result);

    return result;
}

void ddrhook1_master_insert_hooks(HMODULE target)
{
    /* Insert all other hooks here */

    p3io_setupapi_insert_hooks(target);
    monitor_setupapi_insert_hooks(target);
    gfx_insert_hooks(target);
    dinput_init(target);

    /* Insert dynamic loader hooks so that we can hook late-loaded modules */

    hook_table_apply(
        target,
        "kernel32.dll",
        master_kernel32_syms,
        lengthof(master_kernel32_syms));

    log_info("Inserted dynamic loader hooks into %p", target);
}

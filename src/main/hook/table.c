#include <windows.h>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "hook/pe.h"
#include "hook/peb.h"
#include "hook/table.h"

static const char apiset_prefix[] = "api-ms-win-core-";
static const size_t apiset_prefix_len = sizeof(apiset_prefix) - 1;

static void hook_table_apply_to_all(
        const char *depname,
        const struct hook_symbol *syms,
        size_t nsyms);

static void hook_table_apply_to_iid(
        HMODULE target,
        const pe_iid_t *iid,
        const struct hook_symbol *syms,
        size_t nsyms);

static bool hook_table_match_module(
        HMODULE target,
        const char *iid_name,
        const char *depname);

static bool hook_table_match_proc(
        const struct pe_iat_entry *iate,
        const struct hook_symbol *sym);

static void hook_table_apply_to_all(
        const char *depname,
        const struct hook_symbol *syms,
        size_t nsyms)
{
    const peb_dll_t *dll;
    HMODULE pe;

    for (   dll = peb_dll_get_first() ;
            dll != NULL ;
            dll = peb_dll_get_next(dll)) {
        pe = peb_dll_get_base(dll);

        if (pe == NULL) {
            continue; /* ?? Happens sometimes. */
        }

        hook_table_apply(pe, depname, syms, nsyms);
    }
}

void hook_table_apply(
        HMODULE target,
        const char *depname,
        const struct hook_symbol *syms,
        size_t nsyms)
{
    const pe_iid_t *iid;
    const char *iid_name;

    assert(depname != NULL);
    assert(syms != NULL || nsyms == 0);

    if (target == NULL) {
        /*  Call out, which will then call us back repeatedly. Awkward, but
            viewed from the outside it's good for usability. */

        hook_table_apply_to_all(depname, syms, nsyms);
    } else {
        for (   iid = pe_iid_get_first(target) ;
                iid != NULL ;
                iid = pe_iid_get_next(target, iid)) {
            iid_name = pe_iid_get_name(target, iid);

            if (hook_table_match_module(target, iid_name, depname)) {
                hook_table_apply_to_iid(target, iid, syms, nsyms);
            }
        }
    }
}

static void hook_table_apply_to_iid(
        HMODULE target,
        const pe_iid_t *iid,
        const struct hook_symbol *syms,
        size_t nsyms)
{
    struct pe_iat_entry iate;
    size_t i;
    size_t j;
    const struct hook_symbol *sym;

    i = 0;

    while (pe_iid_get_iat_entry(target, iid, i++, &iate) == S_OK) {
        for (j = 0 ; j < nsyms ; j++) {
            sym = &syms[j];

            if (hook_table_match_proc(&iate, sym)) {
                if (sym->link != NULL && *sym->link == NULL) {
                    *sym->link = *iate.ppointer;
                }

                pe_patch(iate.ppointer, &sym->patch, sizeof(sym->patch));
            }
        }
    }
}

static bool hook_table_match_module(
        HMODULE target,
        const char *iid_name,
        const char *depname)
{
    HMODULE kernel32;
    int result;

    /* OK, first do a straightforward match on the imported DLL name versus
       the hook table DLL name. If it succeeds then we're done. */

    result = _stricmp(iid_name, depname);

    if (result == 0) {
        return true;
    }

    /* If it failed then we have to check if this hook table targets kernel32.
       We have to do some special processing around API sets in that case, so
       stop here if kernel32 is not the subject of this hook table. */

    if (_stricmp(depname, "kernel32.dll") != 0) {
        return false;
    }

    /* There isn't really any good test for whether a DLL import wants a
       concrete DLL or an abstract DLL providing a particular Windows API-set,
       so we use a hacky check against the prefix. If the imported DLL name
       looks like an apiset then we'll allow kernel32 hook tables to apply. */

    result = _strnicmp(iid_name, apiset_prefix, apiset_prefix_len);

    if (result != 0) {
        return false;
    }

    /* ... EXCEPT for the case where we're hooking all DLLs loaded into the
       process and we are currently examining kernel32 itself. In that case
       there's some weird reference loops issues I don't entirely understand
       right now. To avoid those, we just don't apply kernel32 hook tables to
       kernel32 itself. */

    kernel32 = GetModuleHandleW(L"kernel32.dll");

    if (target == kernel32) {
        return false;
    }

    return true;
}

static bool hook_table_match_proc(
        const struct pe_iat_entry *iate,
        const struct hook_symbol *sym)
{
    if (    sym->name != NULL &&
            iate->name != NULL &&
            strcmp(sym->name, iate->name) == 0) {
        return true;
    }

    if (sym->ordinal != 0 && sym->ordinal == iate->ordinal) {
        return true;
    }

    return false;
}

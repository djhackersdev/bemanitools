#include <windows.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "hook/pe.h"
#include "hook/peb.h"
#include "hook/table.h"

#include "util/mem.h"

static void hook_table_apply_to_all(
        const char *depname,
        const struct hook_symbol *syms,
        size_t nsyms);

static void hook_table_apply_to_iid(
        HMODULE target,
        const pe_iid_t *iid,
        const struct hook_symbol *syms,
        size_t nsyms);

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

    for (dll = peb_dll_get_first()
            ; dll != NULL
            ; dll = peb_dll_get_next(dll)) {
        pe = peb_dll_get_base(dll);

        if (pe == NULL) {
            /* wtf? */
            continue;
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

    if (target == NULL) {
        /*  Call out, which will then call us back repeatedly. Awkward, but
            viewed from the outside it's good for usability. */

        hook_table_apply_to_all(depname, syms, nsyms);
    } else {
        for (   iid = pe_iid_get_first(target) ;
                iid != NULL ;
                iid = pe_iid_get_next(target, iid)) {
            iid_name = pe_iid_get_name(target, iid);

            if (_stricmp(iid_name, depname) == 0) {
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

    while (pe_iid_get_iat_entry(target, iid, i++, &iate)) {
        for (j = 0 ; j < nsyms ; j++) {
            sym = &syms[j];

            if (hook_table_match_proc(&iate, sym)) {
                if (sym->link != NULL && *sym->link == NULL) {
                    *sym->link = *iate.ppointer;
                }

                pe_patch_pointer(iate.ppointer, sym->patch);
            }
        }
    }
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


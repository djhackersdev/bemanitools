#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hook/pe.h"

static void *pe_offset(void *ptr, size_t off);
static const void *pe_offsetc(const void *ptr, size_t off);
static const IMAGE_NT_HEADERS *pe_get_nt_header(HMODULE pe);

static void *pe_offset(void *ptr, size_t off)
{
    uint8_t *base;

    if (off == 0) {
        return NULL;
    }

    base = ptr;

    return base + off;
}

static const void *pe_offsetc(const void *ptr, size_t off)
{
    const uint8_t *base;

    if (off == 0) {
        return NULL;
    }

    base = ptr;

    return base + off;
}

static const IMAGE_NT_HEADERS *pe_get_nt_header(HMODULE pe)
{
    const IMAGE_DOS_HEADER *dh;
    const IMAGE_NT_HEADERS *nth;

    dh = (IMAGE_DOS_HEADER *) pe;
    nth = pe_offsetc(pe, dh->e_lfanew);

    return nth;
}

const pe_iid_t *pe_iid_get_first(HMODULE pe)
{
    const IMAGE_NT_HEADERS *nth;
    const IMAGE_DATA_DIRECTORY *idd;
    const IMAGE_IMPORT_DESCRIPTOR *iid;

    assert(pe != NULL);

    nth = pe_get_nt_header(pe);
    idd = &nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    iid = pe_offsetc(pe, idd->VirtualAddress);

    if (iid == NULL || iid->Name == 0) {
        return NULL;
    }

    return iid;
}

const char *pe_iid_get_name(HMODULE pe, const pe_iid_t *iid)
{
    assert(pe != NULL);
    assert(iid != NULL);

    return pe_offsetc(pe, iid->Name);
}

const pe_iid_t *pe_iid_get_next(HMODULE pe, const pe_iid_t *iid)
{
    const IMAGE_IMPORT_DESCRIPTOR *iid_next;

    assert(pe != NULL);
    assert(iid != NULL);

    iid_next = iid + 1;

    if (iid_next->Name != 0) {
        return iid_next;
    } else {
        return NULL;
    }
}

HRESULT pe_iid_get_iat_entry(
        HMODULE pe,
        const pe_iid_t *iid,
        size_t n,
        struct pe_iat_entry *entry)
{
    const IMAGE_IMPORT_BY_NAME *import;
    intptr_t *import_rvas;
    void **pointers;

    assert(pe != NULL);
    assert(iid != NULL);
    assert(entry != NULL);

    import_rvas = pe_offset(pe, iid->OriginalFirstThunk);

    if (import_rvas[n] == 0) {
        /* End of imports */
        memset(entry, 0, sizeof(*entry));

        return S_FALSE;
    }

    if (import_rvas[n] & INTPTR_MIN) {
        /* Ordinal import */
        entry->name = NULL;
        entry->ordinal = (uint16_t) import_rvas[n];
    } else {
        /* Named import */
        import = pe_offsetc(pe, import_rvas[n]);
        entry->name = (const char *) import->Name; /* Not an RVA */
        entry->ordinal = 0;
    }

    pointers = pe_offset(pe, iid->FirstThunk);
    entry->ppointer = &pointers[n];

    return S_OK;
}

void *pe_get_export(HMODULE pe, const char *name, uint16_t ord)
{
    const IMAGE_NT_HEADERS *nth;
    const IMAGE_DATA_DIRECTORY *idd;
    const IMAGE_EXPORT_DIRECTORY *ied;
    const uint32_t *name_rvas;
    const uint32_t *target_rvas;
    const char *name_va;
    DWORD i;

    assert(pe != NULL);

    nth = pe_get_nt_header(pe);
    idd = &nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    ied = pe_offsetc(pe, idd->VirtualAddress);

    name_rvas = pe_offsetc(pe, ied->AddressOfNames);
    target_rvas = pe_offsetc(pe, ied->AddressOfFunctions);

    if (name != NULL) {
        for (i = 0 ; i < ied->NumberOfNames ; i++) {
            if (name_rvas[i] == 0) {
                /* Ordinal-only export, cannot match against this */
                continue;
            }

            name_va = pe_offsetc(pe, name_rvas[i]);

            if (strcmp(name_va, name) != 0) {
                /* Name did not match */
                continue;
            }

            return pe_offset(pe, target_rvas[i]);
        }

        return NULL;
    } else if (ord - ied->Base < ied->NumberOfFunctions) {
        return pe_offset(pe, target_rvas[ord - ied->Base]);
    } else {
        return NULL;
    }
}

void *pe_get_entry_point(HMODULE pe)
{
    const IMAGE_NT_HEADERS *nth;

    assert(pe != NULL);

    nth = pe_get_nt_header(pe);

    return pe_offset(pe, nth->OptionalHeader.AddressOfEntryPoint);
}

HRESULT pe_patch(void *dest, const void *src, size_t nbytes)
{
    DWORD old_protect;
    BOOL ok;

    assert(dest != NULL);
    assert(src != NULL);

    ok = VirtualProtect(
            dest,
            nbytes,
            PAGE_EXECUTE_READWRITE,
            &old_protect);

    if (!ok) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    memcpy(dest, src, nbytes);

    ok = VirtualProtect(
            dest,
            nbytes,
            old_protect,
            &old_protect);

    if (!ok) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

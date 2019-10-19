#include <windows.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hook/pe.h"

#include "util/log.h"

typedef BOOL(WINAPI *dll_main_t)(HMODULE, uint32_t, void *);

static const IMAGE_NT_HEADERS *pe_get_nt_header(HMODULE pe);
static uint32_t
pe_get_virtual_size(const IMAGE_SECTION_HEADER *sh, int nsections);
static void *pe_offset(void *ptr, size_t off);
static const void *pe_offsetc(const void *ptr, size_t off);

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

static uint32_t
pe_get_virtual_size(const IMAGE_SECTION_HEADER *sh, int nsections)
{
    uint32_t sec_end;
    uint32_t size;
    int i;

    size = 0;

    for (i = 0; i < nsections; i++) {
        sec_end = sh[i].VirtualAddress + sh[i].Misc.VirtualSize;

        if (size < sec_end) {
            size = sec_end;
        }
    }

    return size;
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
    const IMAGE_IMPORT_DESCRIPTOR *iid;

    nth = pe_get_nt_header(pe);
    iid = pe_offsetc(
        pe,
        nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
            .VirtualAddress);

    if (iid == NULL || iid->Name == 0) {
        return NULL;
    }

    return iid;
}

const char *pe_iid_get_name(HMODULE pe, const pe_iid_t *iid)
{
    return pe_offsetc(pe, iid->Name);
}

const pe_iid_t *pe_iid_get_next(HMODULE pe, const pe_iid_t *iid)
{
    const IMAGE_IMPORT_DESCRIPTOR *iid_next;

    iid_next = iid + 1;

    if (iid_next->Name != 0) {
        return iid_next;
    } else {
        return NULL;
    }
}

bool pe_iid_get_iat_entry(
    HMODULE pe, const pe_iid_t *iid, size_t n, struct pe_iat_entry *entry)
{
    const IMAGE_IMPORT_BY_NAME *import;
    uintptr_t *import_rvas;
    void **pointers;

    if (iid->OriginalFirstThunk == 0) {
        log_warning("OriginalFirstThunk == 0");
    }

    import_rvas = pe_offset(pe, iid->OriginalFirstThunk);

    if (import_rvas[n] == 0) {
        /* End of imports */
        entry->name = NULL;
        entry->ordinal = 0;
        entry->ppointer = NULL;

        return false;
    } else if (import_rvas[n] & INTPTR_MIN) {
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

    return true;
}

void pe_patch_pointer(void **ppointer, void *new_value)
{
    DWORD old_protect;

    VirtualProtect(
        ppointer, sizeof(void *), PAGE_EXECUTE_READWRITE, &old_protect);
    *ppointer = new_value;
    VirtualProtect(ppointer, sizeof(void *), old_protect, &old_protect);
}

HMODULE
pe_explode(const uint8_t *bytes, uint32_t nbytes)
{
    HMODULE base;
    const IMAGE_DOS_HEADER *dh;
    const IMAGE_NT_HEADERS *nth;
    const IMAGE_SECTION_HEADER *sh;
    uint32_t virtual_size;
    uint32_t vflags;
    int i;

    dh = (IMAGE_DOS_HEADER *) bytes;
    nth = pe_offsetc(bytes, dh->e_lfanew);
    sh = pe_offsetc(bytes, dh->e_lfanew + sizeof(*nth));

    virtual_size = pe_get_virtual_size(sh, nth->FileHeader.NumberOfSections);
    base = (HMODULE) VirtualAlloc(
        (void *) nth->OptionalHeader.ImageBase,
        virtual_size,
        MEM_RESERVE,
        PAGE_NOACCESS);

    if (base == NULL) {
        /* Try again, allowing any base address */
        base = (HMODULE) VirtualAlloc(
            NULL, virtual_size, MEM_RESERVE, PAGE_NOACCESS);

        if (base == NULL) {
            /* Aargh */
            log_fatal(
                "Failed to VirtualAlloc %#x bytes of address space",
                virtual_size);
        }
    }

    log_misc(
        "Exploding PE, base %p actual %p",
        (void *) nth->OptionalHeader.ImageBase,
        base);

    /* Commit header region */
    VirtualAlloc(
        (void *) base,
        nth->OptionalHeader.SizeOfHeaders,
        MEM_COMMIT,
        PAGE_READWRITE);

    memcpy(base, dh, sizeof(*dh));
    memcpy(pe_offset(base, dh->e_lfanew), nth, sizeof(*nth));
    memcpy(
        pe_offset(base, dh->e_lfanew + sizeof(*nth)),
        sh,
        sizeof(*sh) * nth->FileHeader.NumberOfSections);

    for (i = 0; i < nth->FileHeader.NumberOfSections; i++) {
        vflags = sh[i].Characteristics & 0x20000000 ? PAGE_EXECUTE_READWRITE :
                                                      PAGE_READWRITE;

        VirtualAlloc(
            pe_offset(base, sh[i].VirtualAddress),
            sh[i].Misc.VirtualSize,
            MEM_COMMIT,
            vflags);

        memcpy(
            pe_offset(base, sh[i].VirtualAddress),
            pe_offsetc(bytes, sh[i].PointerToRawData),
            sh[i].SizeOfRawData);
    }

    return base;
}

void pe_relocate(HMODULE pe)
{
    const IMAGE_NT_HEADERS *nth;
    const IMAGE_DATA_DIRECTORY *dde;
    const IMAGE_BASE_RELOCATION *chunk;
    intptr_t delta_va;
    const uint16_t *reloc;
    uintptr_t *addr_ptr;

    nth = pe_get_nt_header(pe);
    delta_va = (intptr_t) pe - nth->OptionalHeader.ImageBase;
    dde = nth->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_BASERELOC;

    for (chunk = pe_offsetc(pe, dde->VirtualAddress);
         (void *) chunk < pe_offsetc(pe, dde->VirtualAddress + dde->Size);
         chunk = pe_offsetc(chunk, chunk->SizeOfBlock)) {
        for (reloc = (uint16_t *) (chunk + 1);
             (void *) reloc < pe_offsetc(chunk, chunk->SizeOfBlock);
             reloc++) {
            if (*reloc >> 12 == IMAGE_REL_BASED_HIGHLOW) {
                addr_ptr =
                    pe_offset(pe, chunk->VirtualAddress + (*reloc & 0x0FFF));
                *addr_ptr += delta_va;
            }
        }
    }
}

void *pe_get_export(HMODULE pe, const char *name, uint16_t ord)
{
    const IMAGE_NT_HEADERS *nth;
    const IMAGE_EXPORT_DIRECTORY *ied;
    const uint32_t *name_rvas;
    const uint32_t *target_rvas;
    DWORD i;

    nth = pe_get_nt_header(pe);
    ied = pe_offsetc(
        pe,
        nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
            .VirtualAddress);

    name_rvas = pe_offsetc(pe, ied->AddressOfNames);
    target_rvas = pe_offsetc(pe, ied->AddressOfFunctions);

    if (name != NULL) {
        for (i = 0; i < ied->NumberOfNames; i++) {
            if (name_rvas[i] != 0 &&
                strcmp(pe_offsetc(pe, name_rvas[i]), name) == 0) {
                return pe_offset(pe, target_rvas[i]);
            }
        }

        return NULL;
    } else if (ord - ied->Base < ied->NumberOfFunctions) {
        return pe_offset(pe, target_rvas[ord - ied->Base]);
    } else {
        return NULL;
    }
}

BOOL pe_call_dll_main(HMODULE pe, uint32_t reason, void *ctx)
{
    const IMAGE_NT_HEADERS *nth;
    dll_main_t dll_main;

    nth = pe_get_nt_header(pe);
    dll_main = pe_offset(pe, nth->OptionalHeader.AddressOfEntryPoint);

    return dll_main(pe, reason, ctx);
}

#ifndef HOOK_PE_H
#define HOOK_PE_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

typedef IMAGE_IMPORT_DESCRIPTOR pe_iid_t;

struct pe_iat_entry {
    const char *name;
    uint16_t ordinal;
    void **ppointer;
};

const pe_iid_t *pe_iid_get_first(HMODULE pe);
const char *pe_iid_get_name(HMODULE pe, const pe_iid_t *iid);
const pe_iid_t *pe_iid_get_next(HMODULE pe, const pe_iid_t *iid);
bool pe_iid_get_iat_entry(HMODULE pe, const pe_iid_t *iid, size_t n,
        struct pe_iat_entry *entry);
void *pe_get_export(HMODULE pe, const char *name, uint16_t ord);
BOOL pe_call_dll_main(HMODULE pe, uint32_t reason, void *ctx);

void pe_patch_pointer(void **ppointer, void *new_value);

HMODULE pe_explode(const uint8_t *bytes, uint32_t nbytes);
void pe_relocate(HMODULE pe);

#endif

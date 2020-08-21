#pragma once

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>
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
HRESULT pe_iid_get_iat_entry(
        HMODULE pe,
        const pe_iid_t *iid,
        size_t n,
        struct pe_iat_entry *entry);
void *pe_get_export(HMODULE pe, const char *name, uint16_t ord);
void *pe_get_entry_point(HMODULE pe);
HRESULT pe_patch(void *dest, const void *src, size_t nbytes);

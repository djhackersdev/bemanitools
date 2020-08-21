#pragma once

#include <windows.h>
#include <winternl.h>

typedef LDR_DATA_TABLE_ENTRY peb_dll_t;

const peb_dll_t *peb_dll_get_first(void);
const peb_dll_t *peb_dll_get_next(const peb_dll_t *dll);
HMODULE peb_dll_get_base(const peb_dll_t *dll);
char *peb_dll_dup_name(const peb_dll_t *dll);

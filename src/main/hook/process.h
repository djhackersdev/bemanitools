#pragma once

#include <windows.h>

typedef DWORD (CALLBACK *process_entry_t)(void);

HRESULT process_hijack_startup(
        process_entry_t new_entry,
        process_entry_t *orig_entry);

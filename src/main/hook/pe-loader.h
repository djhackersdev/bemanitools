#pragma once

#include <windows.h>

#include <stdlib.h>

HRESULT pe_loader_load_from_file(const char* name, HMODULE* module);
HRESULT pe_loader_load_from_mem(void* data, size_t size, HMODULE* module);
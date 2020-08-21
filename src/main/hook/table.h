#pragma once

#include <windows.h>

#include <stddef.h>
#include <stdint.h>

struct hook_symbol {
    const char *name;
    uint16_t ordinal;
    void *patch;
    void **link;
};

void hook_table_apply(
        HMODULE target,
        const char *depname,
        const struct hook_symbol *syms,
        size_t nsyms);

void hook_table_revert(
        HMODULE target,
        const char *depname,
        const struct hook_symbol *syms,
        size_t nsyms);

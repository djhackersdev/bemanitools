#define LOG_MODULE "mem"

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"

void *xcalloc(size_t nbytes)
{
    void *mem;

    mem = calloc(nbytes, 1);

    if (mem == NULL) {
        log_fatal("xcalloc(%u) failed", (uint32_t) nbytes);

        return NULL;
    }

    return mem;
}

void *xmalloc(size_t nbytes)
{
    void *mem;

    mem = malloc(nbytes);

    if (mem == NULL) {
        log_fatal("xmalloc(%u) failed", (uint32_t) nbytes);

        return NULL;
    }

    return mem;
}

void *xrealloc(void *mem, size_t nbytes)
{
    void *newmem;

    newmem = realloc(mem, nbytes);

    if (newmem == NULL) {
        log_fatal("xrealloc(%p, %u) failed", mem, (uint32_t) nbytes);

        return NULL;
    }

    return newmem;
}

bool mem_nop(size_t mem_offset, size_t length)
{
    DWORD oldProt;

    if (VirtualProtect(
            (void *) (mem_offset), length, PAGE_EXECUTE_READWRITE, &oldProt) ==
        0) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        ((uint8_t *) mem_offset)[i] = 0x90;
    }

    if (VirtualProtect((void *) (mem_offset), length, oldProt, &oldProt) == 0) {
        return false;
    }

    return true;
}

void *mem_find_signiture(
    const uint8_t *sig,
    uint32_t sig_len,
    int32_t sig_offset,
    void *start_addr,
    void *end_addr,
    int32_t alignment)
{
    size_t pos;

    for (pos = (size_t) start_addr; pos < (size_t) end_addr; pos += alignment) {
        if (!memcmp((void *) pos, sig, sig_len)) {
            return (void *) (pos + sig_offset);
        }
    }

    return NULL;
}
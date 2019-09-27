#ifndef UTIL_MEM_H
#define UTIL_MEM_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void *xcalloc(size_t nbytes);
void *xmalloc(size_t nbytes);
void *xrealloc(void *mem, size_t nbytes);
bool mem_nop(size_t mem_offset, size_t length);

/**
 * Scan a memory area limited by a start and end address for a signiture.
 *
 * @param sig Buffer with signiture to search for
 * @param sig_len Length of signiture
 * @param sig_offset Offset that gets added to the address where the signiture
 *                   starts (if found)
 * @param start_addr Pointer to the address to start searching at
 * @param end_addr Pointer to the address to stop searching at
 * @param alignment Aligns the search signiture, i.e. search in steps of
 *                  X bytes
 * @return If found, pointer to memory where the signiture was found (first
 *         occurance) with the signiture offset added, NULL otherwise
 */
void* mem_find_signiture(const uint8_t* sig, uint32_t sig_len,
    int32_t sig_offset, void* start_addr, void* end_addr, int32_t alignment);

#endif

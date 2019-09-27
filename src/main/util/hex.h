#ifndef UTIL_HEX_H
#define UTIL_HEX_H

#include <stdbool.h>
#include <stddef.h>

bool hex_decode(void *bytes, size_t nbytes, const char *chars,
        size_t nchars);
void hex_encode_uc(const void *bytes, size_t nbytes, char *chars,
        size_t nchars);
void hex_encode_lc(const void *bytes, size_t nbytes, char *chars,
        size_t nchars);

#endif

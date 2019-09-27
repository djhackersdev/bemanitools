#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "util/hex.h"
#include "util/log.h"

static bool hex_decode_nibble(char c, uint8_t *nibble)
{
    if (c >= '0' && c <= '9') {
        *nibble = c - '0';
    } else if (c >= 'A' && c <= 'F') {
        *nibble = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        *nibble = c - 'a' + 10;
    } else {
        return false;
    }

    return true;
}

bool hex_decode(void *ptr, size_t nbytes, const char *chars, size_t nchars)
{
    uint8_t *bytes = ptr;

    uint32_t i;
    uint8_t hi;
    uint8_t lo;

    log_assert(nchars <= 2 * nbytes);
    log_assert(nchars % 2 == 0);

    for (i = 0 ; i < nchars / 2; i++) {
        if (    !hex_decode_nibble(chars[2 * i + 0], &hi) ||
                !hex_decode_nibble(chars[2 * i + 1], &lo)) {
            return false;
        }

        bytes[i] = (hi << 4) | lo;
    }

    return true;
}

static void hex_encode(const void *ptr, size_t nbytes, char *chars,
        size_t nchars, const char *digits)
{
    const uint8_t *bytes = ptr;

    size_t i;

    log_assert(nchars >= 2 * nbytes + 1);

    for (i = 0 ; i < nbytes ; i++) {
        chars[i * 2 + 0] = digits[bytes[i] >> 4];
        chars[i * 2 + 1] = digits[bytes[i] & 15];
    }

    chars[nbytes * 2] = '\0';
}

void hex_encode_lc(const void *bytes, size_t nbytes, char *chars, size_t nchars)
{
    hex_encode(bytes, nbytes, chars, nchars, "0123456789abcdef");
}

void hex_encode_uc(const void *bytes, size_t nbytes, char *chars, size_t nchars)
{
    hex_encode(bytes, nbytes, chars, nchars, "0123456789ABCDEF");
}


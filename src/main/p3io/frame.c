#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "p3io/frame.h"

#include "util/iobuf.h"
#include "util/log.h"

#define P3IO_FRAME_SOF 0xAA
#define P3IO_FRAME_ESCAPE 0xFF

HRESULT
p3io_frame_encode(struct iobuf *dest, const void *ptr, size_t nbytes)
{
    const uint8_t *bytes;
    uint8_t b;
    size_t i;

    log_assert(dest != NULL);
    log_assert(ptr != NULL);

    bytes = ptr;

    if (dest->pos >= dest->nbytes) {
        goto trunc;
    }

    dest->bytes[dest->pos++] = P3IO_FRAME_SOF;

    for (i = 0; i < nbytes; i++) {
        b = bytes[i];

        if (b == P3IO_FRAME_SOF || b == P3IO_FRAME_ESCAPE) {
            if (dest->pos + 1 >= dest->nbytes) {
                goto trunc;
            }

            dest->bytes[dest->pos++] = P3IO_FRAME_ESCAPE;
            dest->bytes[dest->pos++] = ~b;
        } else {
            if (dest->pos >= dest->nbytes) {
                goto trunc;
            }

            dest->bytes[dest->pos++] = b;
        }
    }

    if (i < nbytes) {
        goto trunc;
    }

    return S_OK;

trunc:
    return E_FAIL;
}

HRESULT
p3io_frame_decode(struct iobuf *dest, struct const_iobuf *src)
{
    bool escape;
    uint8_t b;

    log_assert(dest != NULL);
    log_assert(src != NULL);

    if (src->pos >= src->nbytes || src->bytes[src->pos] != P3IO_FRAME_SOF) {
        return E_FAIL;
    }

    src->pos++;
    escape = false;

    while (src->pos < src->nbytes) {
        if (dest->pos >= dest->nbytes) {
            return E_FAIL;
        }

        b = src->bytes[src->pos++];

        if (b == P3IO_FRAME_SOF) {
            return E_FAIL;
        } else if (b == P3IO_FRAME_ESCAPE) {
            if (escape) {
                return E_FAIL;
            }

            escape = true;
        } else {
            if (escape) {
                dest->bytes[dest->pos++] = ~b;
            } else {
                dest->bytes[dest->pos++] = b;
            }

            escape = false;
        }
    }

    if (escape) {
        return E_FAIL;
    }

    return S_OK;
}

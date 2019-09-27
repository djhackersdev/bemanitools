#ifndef UTIL_IOBUF_H
#define UTIL_IOBUF_H

#include <stddef.h>
#include <stdint.h>

struct iobuf {
    uint8_t *bytes;
    size_t nbytes;
    size_t pos;
};

struct const_iobuf {
    const uint8_t *bytes;
    size_t nbytes;
    size_t pos;
};

size_t iobuf_move(struct iobuf *dest, struct const_iobuf *src);

void iobuf_flip(struct const_iobuf *dest, const struct iobuf *src);

void iobuf_log(struct iobuf* buffer, const char* tag);

void iobuf_log_const(struct const_iobuf* buffer, const char* tag);

#endif

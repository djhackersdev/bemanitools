#include <assert.h>
#include <string.h>

#include "hook/iobuf.h"

void iobuf_flip(struct const_iobuf *child, struct iobuf *parent)
{
    assert(child != NULL);
    assert(parent != NULL);

    child->bytes = parent->bytes;
    child->pos = 0;
    child->nbytes = parent->pos;
}

size_t iobuf_move(struct iobuf *dest, struct const_iobuf *src)
{
    size_t dest_avail;
    size_t src_avail;
    size_t chunksz;

    assert(dest != NULL);
    assert(dest->bytes != NULL || dest->nbytes == 0);
    assert(dest->pos <= dest->nbytes);

    assert(src != NULL);
    assert(src->bytes != NULL || src->nbytes == 0);
    assert(src->pos <= src->nbytes);

    dest_avail = dest->nbytes - dest->pos;
    src_avail = src->nbytes - src->pos;
    chunksz = dest_avail < src_avail ? dest_avail : src_avail;

    memcpy(&dest->bytes[dest->pos], &src->bytes[src->pos], chunksz);

    dest->pos += chunksz;
    src->pos += chunksz;

    return chunksz;
}

size_t iobuf_shift(struct iobuf *dest, struct iobuf *src)
{
    struct const_iobuf span;

    assert(dest != NULL);
    assert(src != NULL);

    iobuf_flip(&span, src);
    iobuf_move(dest, &span);

    memmove(src->bytes, &src->bytes[span.pos], span.nbytes - span.pos);
    src->pos -= span.pos;

    return span.pos;
}

HRESULT iobuf_read(struct const_iobuf *src, void *bytes, size_t nbytes)
{
    assert(src != NULL);
    assert(bytes != NULL || nbytes == 0);

    if (src->pos + nbytes > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    memcpy(bytes, &src->bytes[src->pos], nbytes);
    src->pos += nbytes;

    return S_OK;
}

HRESULT iobuf_read_8(struct const_iobuf *src, uint8_t *out)
{
    assert(src != NULL);
    assert(out != NULL);

    if (src->pos + sizeof(uint8_t) > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    *out = src->bytes[src->pos++];

    return S_OK;
}

HRESULT iobuf_read_be16(struct const_iobuf *src, uint16_t *out)
{
    uint16_t value;

    assert(src != NULL);
    assert(out != NULL);

    if (src->pos + sizeof(uint16_t) > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    value  = src->bytes[src->pos++] << 8;
    value |= src->bytes[src->pos++];

    *out = value;

    return S_OK;
}

HRESULT iobuf_read_be32(struct const_iobuf *src, uint32_t *out)
{
    uint32_t value;

    assert(src != NULL);
    assert(out != NULL);

    if (src->pos + sizeof(uint32_t) > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    value  = src->bytes[src->pos++] << 24;
    value |= src->bytes[src->pos++] << 16;
    value |= src->bytes[src->pos++] << 8;
    value |= src->bytes[src->pos++];

    *out = value;

    return S_OK;
}

HRESULT iobuf_read_be64(struct const_iobuf *src, uint64_t *out)
{
    uint64_t value;

    assert(src != NULL);
    assert(out != NULL);

    if (src->pos + sizeof(uint64_t) > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    value  = ((uint64_t) src->bytes[src->pos++]) << 56;
    value |= ((uint64_t) src->bytes[src->pos++]) << 48;
    value |= ((uint64_t) src->bytes[src->pos++]) << 40;
    value |= ((uint64_t) src->bytes[src->pos++]) << 32;
    value |= ((uint64_t) src->bytes[src->pos++]) << 24;
    value |= ((uint64_t) src->bytes[src->pos++]) << 16;
    value |= ((uint64_t) src->bytes[src->pos++]) << 8;
    value |= ((uint64_t) src->bytes[src->pos++]);

    *out = value;

    return S_OK;
}

HRESULT iobuf_read_le16(struct const_iobuf *src, uint16_t *out)
{
    uint16_t value;

    assert(src != NULL);
    assert(out != NULL);

    if (src->pos + sizeof(uint16_t) > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    value  = src->bytes[src->pos++];
    value |= src->bytes[src->pos++] << 8;

    *out = value;

    return S_OK;
}

HRESULT iobuf_read_le32(struct const_iobuf *src, uint32_t *out)
{
    uint32_t value;

    assert(src != NULL);
    assert(out != NULL);

    if (src->pos + sizeof(uint32_t) > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    value  = src->bytes[src->pos++];
    value |= src->bytes[src->pos++] << 8;
    value |= src->bytes[src->pos++] << 16;
    value |= src->bytes[src->pos++] << 24;

    *out = value;

    return S_OK;
}

HRESULT iobuf_read_le64(struct const_iobuf *src, uint64_t *out)
{
    uint64_t value;

    assert(src != NULL);
    assert(out != NULL);

    if (src->pos + sizeof(uint64_t) > src->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    value  = ((uint64_t) src->bytes[src->pos++]);
    value |= ((uint64_t) src->bytes[src->pos++]) << 8;
    value |= ((uint64_t) src->bytes[src->pos++]) << 16;
    value |= ((uint64_t) src->bytes[src->pos++]) << 24;
    value |= ((uint64_t) src->bytes[src->pos++]) << 32;
    value |= ((uint64_t) src->bytes[src->pos++]) << 40;
    value |= ((uint64_t) src->bytes[src->pos++]) << 48;
    value |= ((uint64_t) src->bytes[src->pos++]) << 56;

    *out = value;

    return S_OK;
}

HRESULT iobuf_write(struct iobuf *dest, const void *bytes, size_t nbytes)
{
    assert(dest != NULL);
    assert(bytes != NULL || nbytes == 0);

    if (dest->pos + nbytes > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    memcpy(&dest->bytes[dest->pos], bytes, nbytes);
    dest->pos += nbytes;

    return S_OK;
}

HRESULT iobuf_write_8(struct iobuf *dest, uint8_t value)
{
    assert(dest != NULL);

    if (dest->pos + sizeof(uint8_t) > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    dest->bytes[dest->pos++] = value;

    return S_OK;
}

HRESULT iobuf_write_be16(struct iobuf *dest, uint16_t value)
{
    assert(dest != NULL);

    if (dest->pos + sizeof(uint16_t) > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    dest->bytes[dest->pos++] = value >> 8;
    dest->bytes[dest->pos++] = value;

    return S_OK;
}

HRESULT iobuf_write_be32(struct iobuf *dest, uint32_t value)
{
    assert(dest != NULL);

    if (dest->pos + sizeof(uint32_t) > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    dest->bytes[dest->pos++] = value >> 24;
    dest->bytes[dest->pos++] = value >> 16;
    dest->bytes[dest->pos++] = value >> 8;
    dest->bytes[dest->pos++] = value;

    return S_OK;
}

HRESULT iobuf_write_be64(struct iobuf *dest, uint64_t value)
{
    assert(dest != NULL);

    if (dest->pos + sizeof(uint64_t) > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    dest->bytes[dest->pos++] = value >> 56;
    dest->bytes[dest->pos++] = value >> 48;
    dest->bytes[dest->pos++] = value >> 40;
    dest->bytes[dest->pos++] = value >> 32;
    dest->bytes[dest->pos++] = value >> 24;
    dest->bytes[dest->pos++] = value >> 16;
    dest->bytes[dest->pos++] = value >> 8;
    dest->bytes[dest->pos++] = value;

    return S_OK;
}

HRESULT iobuf_write_le16(struct iobuf *dest, uint16_t value)
{
    assert(dest != NULL);

    if (dest->pos + sizeof(uint16_t) > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    dest->bytes[dest->pos++] = value;
    dest->bytes[dest->pos++] = value >> 8;

    return S_OK;
}

HRESULT iobuf_write_le32(struct iobuf *dest, uint32_t value)
{
    assert(dest != NULL);

    if (dest->pos + sizeof(uint32_t) > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    dest->bytes[dest->pos++] = value;
    dest->bytes[dest->pos++] = value >> 8;
    dest->bytes[dest->pos++] = value >> 16;
    dest->bytes[dest->pos++] = value >> 24;

    return S_OK;
}

HRESULT iobuf_write_le64(struct iobuf *dest, uint64_t value)
{
    assert(dest != NULL);

    if (dest->pos + sizeof(uint64_t) > dest->nbytes) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    dest->bytes[dest->pos++] = value;
    dest->bytes[dest->pos++] = value >> 8;
    dest->bytes[dest->pos++] = value >> 16;
    dest->bytes[dest->pos++] = value >> 24;
    dest->bytes[dest->pos++] = value >> 32;
    dest->bytes[dest->pos++] = value >> 40;
    dest->bytes[dest->pos++] = value >> 48;
    dest->bytes[dest->pos++] = value >> 56;

    return S_OK;
}

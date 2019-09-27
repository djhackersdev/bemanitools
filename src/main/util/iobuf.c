#define LOG_MODULE "iobuf"

#include <string.h>

#include "util/hex.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/mem.h"

size_t iobuf_move(struct iobuf *dest, struct const_iobuf *src)
{
    size_t dest_avail;
    size_t src_avail;
    size_t chunksz;

    dest_avail = dest->nbytes - dest->pos;
    src_avail = src->nbytes - src->pos;
    chunksz = dest_avail < src_avail ? dest_avail : src_avail;

    memcpy(&dest->bytes[dest->pos], &src->bytes[src->pos], chunksz);

    dest->pos += chunksz;
    src->pos += chunksz;

    return chunksz;
}

void iobuf_flip(struct const_iobuf *dest, const struct iobuf *src)
{
    log_assert(dest != NULL);
    log_assert(src != NULL);

    dest->bytes = src->bytes;
    dest->pos = 0;
    dest->nbytes = src->pos;
}

void iobuf_log(struct iobuf* buffer, const char* tag)
{
    char* str;
    size_t str_len;

    str_len = buffer->nbytes * 2 + 1;
    str = xmalloc(str_len);

    log_misc("[%s] (%d %d)", tag, (uint32_t) buffer->nbytes,
        (uint32_t) buffer->pos);

    hex_encode_uc(buffer->bytes, buffer->nbytes, str, str_len);

    log_misc("[%s]: %s", tag, str);

    free(str);
}

void iobuf_log_const(struct const_iobuf* buffer, const char* tag)
{
    char* str;
    size_t str_len;

    str_len = buffer->nbytes * 2 + 1;
    str = xmalloc(str_len);

    log_misc("[%s] (%d %d)", tag, (uint32_t) buffer->nbytes,
        (uint32_t) buffer->pos);

    hex_encode_uc(buffer->bytes, buffer->nbytes, str, str_len);

    log_misc("[%s]: %s", tag, str);

    free(str);
}

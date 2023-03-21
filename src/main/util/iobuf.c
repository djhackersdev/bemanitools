#define LOG_MODULE "util-iobuf"

#include "util/iobuf.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/mem.h"

void iobuf_log(struct iobuf *buffer, const char *tag)
{
    char *str;
    size_t str_len;

    str_len = buffer->nbytes * 2 + 1;
    str = xmalloc(str_len);

    log_misc(
        "[%s] (%d %d)", tag, (uint32_t) buffer->nbytes, (uint32_t) buffer->pos);

    hex_encode_uc(buffer->bytes, buffer->nbytes, str, str_len);

    log_misc("[%s]: %s", tag, str);

    free(str);
}

void iobuf_log_const(struct const_iobuf *buffer, const char *tag)
{
    char *str;
    size_t str_len;

    str_len = buffer->nbytes * 2 + 1;
    str = xmalloc(str_len);

    log_misc(
        "[%s] (%d %d)", tag, (uint32_t) buffer->nbytes, (uint32_t) buffer->pos);

    hex_encode_uc(buffer->bytes, buffer->nbytes, str, str_len);

    log_misc("[%s]: %s", tag, str);

    free(str);
}

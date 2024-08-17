#define LOG_MODULE "util-iobuf"

#include "iface-core/log.h"

#include "util/hex.h"
#include "util/iobuf.h"
#include "util/mem.h"

void iobuf_log(struct iobuf *buffer, const char *tag)
{
    char *str;
    size_t str_len;

    str_len = buffer->nbytes * 2 + 1;
    str = xmalloc(str_len);

    log_misc(
        "[%s] (nbytes %d, pos %d)",
        tag,
        (uint32_t) buffer->nbytes,
        (uint32_t) buffer->pos);

    hex_encode_uc(buffer->bytes, buffer->nbytes, str, str_len);

    log_misc("full [%s]: %s", tag, str);

    hex_encode_uc(buffer->bytes, buffer->pos, str, str_len);

    log_misc("pos [%s]: %s", tag, str);

    free(str);
}

void iobuf_log_const(struct const_iobuf *buffer, const char *tag)
{
    char *str;
    size_t str_len;

    str_len = buffer->nbytes * 2 + 1;
    str = xmalloc(str_len);

    log_misc(
        "[%s] (nbytes %d, pos %d)",
        tag,
        (uint32_t) buffer->nbytes,
        (uint32_t) buffer->pos);

    hex_encode_uc(buffer->bytes, buffer->nbytes, str, str_len);

    log_misc("full [%s]: %s", tag, str);

    hex_encode_uc(buffer->bytes, buffer->pos, str, str_len);

    log_misc("pos [%s]: %s", tag, str);

    free(str);
}

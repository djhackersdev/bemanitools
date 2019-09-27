#include <stddef.h>

#include "p3io/cmd.h"

#include "util/log.h"

uint8_t p3io_req_cmd(const union p3io_req_any *src)
{
    log_assert(src != NULL);

    /* In requests, the command byte is the first byte after the header. */

    return src->raw[sizeof(struct p3io_hdr)];
}

void p3io_resp_init(
        struct p3io_hdr *dest,
        size_t nbytes,
        const struct p3io_hdr *req)
{
    log_assert(dest != NULL);
    log_assert(req != NULL);
    log_assert(nbytes < 0x100);

    /* Length byte in this packet format counts everything from the length
       byte onwards. The length byte itself occurs at the start of the frame. */

    dest->nbytes = nbytes - 1;
    dest->seq_no = req->seq_no;
}

#include <stddef.h>

#include "core/log.h"

#include "p3io/cmd.h"

uint8_t p3io_get_full_req_size(const union p3io_req_any *req)
{
    /* Length byte in this packet format counts everything from the length
       byte onwards. The length byte itself occurs at the start of the frame. */

    return req->hdr.nbytes + 1;
}

uint8_t p3io_get_full_resp_size(const union p3io_resp_any *resp)
{
    /* Length byte in this packet format counts everything from the length
       byte onwards. The length byte itself occurs at the start of the frame. */

    return resp->hdr.nbytes + 1;
}

void p3io_req_hdr_init(
    struct p3io_hdr *req_hdr, uint8_t seq_no, uint8_t cmd, size_t size)
{
    log_assert(req_hdr != NULL);
    log_assert(size < P3IO_MAX_MESSAGE_SIZE);

    memset(req_hdr, 0, sizeof(struct p3io_hdr));

    /* Length byte in this packet format counts everything from the length
       byte onwards. The length byte itself occurs at the start of the frame. */

    req_hdr->nbytes = size - 1;
    req_hdr->seq_no = seq_no;
    req_hdr->cmd = cmd;
}

void p3io_resp_hdr_init(
    struct p3io_hdr *resp_hdr, size_t nbytes, const struct p3io_hdr *req_hdr)
{
    log_assert(resp_hdr != NULL);
    log_assert(req_hdr != NULL);
    log_assert(nbytes <= P3IO_MAX_MESSAGE_SIZE);

    /* Length byte in this packet format counts everything from the length
       byte onwards. The length byte itself occurs at the start of the frame. */

    resp_hdr->nbytes = nbytes - 1;
    resp_hdr->seq_no = req_hdr->seq_no;
    resp_hdr->cmd = req_hdr->cmd;
}

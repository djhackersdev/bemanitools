#include <stdint.h>

#include "acio/acio.h"

#include "acioemu/addr.h"
#include "acioemu/emu.h"

#include "util/log.h"

void ac_io_emu_cmd_assign_addrs(
        struct ac_io_emu *emu,
        const struct ac_io_message *req,
        uint8_t node_count)
{
    struct ac_io_message resp;
    uint16_t cmd;

    log_assert(emu != NULL);
    log_assert(req != NULL);

    cmd = ac_io_u16(req->cmd.code);

    if (cmd != AC_IO_CMD_ASSIGN_ADDRS) {
        log_warning(
                "Address 0 expects address assignment cmd, got %04x",
                cmd);

        return;
    }

    memset(&resp, 0, sizeof(resp));
    resp.addr = 0;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.count);
    resp.cmd.count = node_count;

    ac_io_emu_response_push(emu, &resp, 0);
}

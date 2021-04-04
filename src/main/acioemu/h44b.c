#define LOG_MODULE "acioemu-h44b"

#include "acioemu/h44b.h"

#include <windows.h> /* for _BitScanForward */

#include <stdint.h>
#include <string.h>

#include "acio/h44b.h"

#include "acioemu/emu.h"

#include "bemanitools/jbio.h"

#include "util/hex.h"

static void ac_io_emu_h44b_cmd_send_version(
    struct ac_io_emu_h44b *h44b, const struct ac_io_message *req);

static void ac_io_emu_h44b_send_status(
    struct ac_io_emu_h44b *h44b,
    const struct ac_io_message *req,
    uint8_t status);

void ac_io_emu_h44b_init(
    struct ac_io_emu_h44b *h44b, struct ac_io_emu *emu, uint8_t unit_no)
{
    memset(h44b, 0, sizeof(*h44b));
    h44b->emu = emu;
    h44b->unit_no = unit_no;
}

void ac_io_emu_h44b_dispatch_request(
    struct ac_io_emu_h44b *h44b, const struct ac_io_message *req)
{
    uint16_t cmd_code;

    cmd_code = ac_io_u16(req->cmd.code);

    switch (cmd_code) {
        case AC_IO_CMD_GET_VERSION:
            log_misc("AC_IO_CMD_GET_VERSION(%d)", req->addr);
            ac_io_emu_h44b_cmd_send_version(h44b, req);

            break;

        case AC_IO_CMD_START_UP:
            log_misc("AC_IO_CMD_START_UP(%d)", req->addr);
            ac_io_emu_h44b_send_status(h44b, req, 0x00);

            break;

        case AC_IO_H44B_CMD_SET_OUTPUTS:
            /* Not using the struct ac_io_h44b_output here */
            for (int i = 0; i < 6; i++) {
                jb_io_set_rgb_led(
                    (enum jb_io_rgb_led) i,
                    req->cmd.raw[i * 3],
                    req->cmd.raw[i * 3 + 1],
                    req->cmd.raw[i * 3 + 2]);
            }

            jb_io_write_lights();

            ac_io_emu_h44b_send_status(h44b, req, 0x00);

            break;

        default:
            log_warning(
                "Unknown ACIO message %04x on h44b node, addr=%d",
                cmd_code,
                req->addr);

            break;
    }
}

static void ac_io_emu_h44b_cmd_send_version(
    struct ac_io_emu_h44b *h44b, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.version);
    resp.cmd.version.type = ac_io_u32(AC_IO_NODE_TYPE_H44B);
    resp.cmd.version.flag = 0x00;
    resp.cmd.version.major = 0x01;
    resp.cmd.version.minor = 0x00;
    resp.cmd.version.revision = 0x02;
    memcpy(
        resp.cmd.version.product_code,
        "H44B",
        sizeof(resp.cmd.version.product_code));
    strncpy(resp.cmd.version.date, __DATE__, sizeof(resp.cmd.version.date));
    strncpy(resp.cmd.version.time, __TIME__, sizeof(resp.cmd.version.time));

    ac_io_emu_response_push(h44b->emu, &resp, 0);
}

static void ac_io_emu_h44b_send_status(
    struct ac_io_emu_h44b *h44b,
    const struct ac_io_message *req,
    uint8_t status)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.status);
    resp.cmd.status = status;

    ac_io_emu_response_push(h44b->emu, &resp, 0);
}

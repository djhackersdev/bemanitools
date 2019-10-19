#define LOG_MODULE "acioemu-hdxs"

#include "acio/acio.h"

#include "acioemu/emu.h"
#include "acioemu/hdxs.h"

#include "util/log.h"

static void ac_io_emu_hdxs_cmd_send_version(
    struct ac_io_emu_hdxs *hdxs, const struct ac_io_message *req);

static void ac_io_emu_hdxs_send_empty(
    struct ac_io_emu_hdxs *hdxs, const struct ac_io_message *req);

static void ac_io_emu_hdxs_send_status(
    struct ac_io_emu_hdxs *hdxs,
    const struct ac_io_message *req,
    uint8_t status);

void ac_io_emu_hdxs_init(struct ac_io_emu_hdxs *hdxs, struct ac_io_emu *emu)
{
    log_assert(hdxs != NULL);
    log_assert(emu != NULL);

    hdxs->emu = emu;
}

void ac_io_emu_hdxs_dispatch_request(
    struct ac_io_emu_hdxs *hdxs, const struct ac_io_message *req)
{
    uint16_t cmd_code;

    log_assert(hdxs != NULL);
    log_assert(req != NULL);

    cmd_code = ac_io_u16(req->cmd.code);

    switch (cmd_code) {
        case AC_IO_CMD_GET_VERSION:
            log_misc("AC_IO_CMD_GET_VERSION(%d)", req->addr);
            ac_io_emu_hdxs_cmd_send_version(hdxs, req);

            break;

        case AC_IO_CMD_START_UP:
            log_misc("AC_IO_CMD_START_UP(%d)", req->addr);
            ac_io_emu_hdxs_send_status(hdxs, req, 0x00);

            break;

        case AC_IO_CMD_CLEAR:
            log_misc("AC_IO_CMD_CLEAR(%d)", req->addr);

        case 0x110:
        case 0x112:
        case 0x128:
            ac_io_emu_hdxs_send_status(hdxs, req, 0x00);

            break;

        case AC_IO_CMD_KEEPALIVE:
            ac_io_emu_hdxs_send_empty(hdxs, req);

            break;

        default:
            log_warning(
                "Unknown ACIO message %04x on HDXS node, addr=%d",
                cmd_code,
                req->addr);

            break;
    }
}

static void ac_io_emu_hdxs_cmd_send_version(
    struct ac_io_emu_hdxs *hdxs, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.version);
    resp.cmd.version.type = ac_io_u32(AC_IO_NODE_TYPE_LED_STRIP);
    resp.cmd.version.flag = 0x00;
    resp.cmd.version.major = 0x01;
    resp.cmd.version.minor = 0x06;
    resp.cmd.version.revision = 0x00;
    memcpy(
        resp.cmd.version.product_code,
        "HDXS",
        sizeof(resp.cmd.version.product_code));
    strncpy(resp.cmd.version.date, __DATE__, sizeof(resp.cmd.version.date));
    strncpy(resp.cmd.version.time, __TIME__, sizeof(resp.cmd.version.time));

    ac_io_emu_response_push(hdxs->emu, &resp, 0);
}

static void ac_io_emu_hdxs_send_empty(
    struct ac_io_emu_hdxs *hdxs, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = 0;

    ac_io_emu_response_push(hdxs->emu, &resp, 0);
}

static void ac_io_emu_hdxs_send_status(
    struct ac_io_emu_hdxs *hdxs,
    const struct ac_io_message *req,
    uint8_t status)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.status);
    resp.cmd.status = status;

    ac_io_emu_response_push(hdxs->emu, &resp, 0);
}

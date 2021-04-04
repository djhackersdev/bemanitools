#define LOG_MODULE "acioemu-iccb"

#include "acioemu/iccb.h"

#include <windows.h> /* for _BitScanForward */

#include <stdint.h>
#include <string.h>

#include "acio/iccb.h"

#include "acioemu/emu.h"

#include "bemanitools/eamio.h"

static void ac_io_emu_iccb_cmd_send_version(
    struct ac_io_emu_iccb *iccb, const struct ac_io_message *req);

static void ac_io_emu_iccb_send_state(
    struct ac_io_emu_iccb *iccb, const struct ac_io_message *req);

static void ac_io_emu_iccb_send_empty(
    struct ac_io_emu_iccb *iccb, const struct ac_io_message *req);

static void ac_io_emu_iccb_send_status(
    struct ac_io_emu_iccb *iccb,
    const struct ac_io_message *req,
    uint8_t status);

void ac_io_emu_iccb_init(
    struct ac_io_emu_iccb *iccb, struct ac_io_emu *emu, uint8_t unit_no)
{
    memset(iccb, 0, sizeof(*iccb));
    iccb->emu = emu;
    iccb->unit_no = unit_no;
}

void ac_io_emu_iccb_dispatch_request(
    struct ac_io_emu_iccb *iccb, const struct ac_io_message *req)
{
    uint16_t cmd_code;

    cmd_code = ac_io_u16(req->cmd.code);

    switch (cmd_code) {
        case AC_IO_CMD_GET_VERSION:
            log_misc("AC_IO_CMD_GET_VERSION(%d)", req->addr);
            ac_io_emu_iccb_cmd_send_version(iccb, req);

            break;

        case AC_IO_CMD_START_UP:
            log_misc("AC_IO_CMD_START_UP(%d)", req->addr);
            ac_io_emu_iccb_send_status(iccb, req, 0x00);

            break;

        case AC_IO_CMD_KEEPALIVE:
            ac_io_emu_iccb_send_empty(iccb, req);

            break;

        case AC_IO_ICCB_CMD_QUEUE_LOOP_START:
            log_misc("AC_IO_CMD_QUEUE_LOOP_START(%d)", req->addr);
            ac_io_emu_iccb_send_status(iccb, req, 0x00);

            break;

        case AC_IO_ICCB_CMD_UNK_0100:
        case AC_IO_ICCB_CMD_UNK_0116:
        case AC_IO_ICCB_CMD_UNK_0120:
            log_misc("AC_IO_ICCB_CMD_UNK_%04X(%d)", cmd_code, req->addr);
            ac_io_emu_iccb_send_status(iccb, req, 0x00);

            break;

        case AC_IO_ICCB_CMD_SLEEP:
            ac_io_emu_iccb_send_status(iccb, req, 0x00);

            break;

        case AC_IO_ICCB_CMD_UNK_135:
            /* log_misc("AC_IO_ICCB_CMD_UNK_135"); */
            ac_io_emu_iccb_send_state(iccb, req);

            break;

        case AC_IO_ICCB_CMD_POLL:
            /* log_misc("AC_IO_ICCB_CMD_POLL"); */
            ac_io_emu_iccb_send_state(iccb, req);

            break;

        case AC_IO_ICCB_CMD_READ_CARD:
            /* log_misc("AC_IO_ICCB_CMD_READ_CARD"); */
            ac_io_emu_iccb_send_state(iccb, req);

            break;

        default:
            log_warning(
                "Unknown ACIO message %04x on ICCB node, addr=%d",
                cmd_code,
                req->addr);

            break;
    }
}

static void ac_io_emu_iccb_cmd_send_version(
    struct ac_io_emu_iccb *iccb, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.version);
    resp.cmd.version.type = ac_io_u32(AC_IO_NODE_TYPE_ICCB);
    resp.cmd.version.flag = 0x00;
    resp.cmd.version.major = 0x01;
    resp.cmd.version.minor = 0x05;
    resp.cmd.version.revision = 0x01;
    memcpy(
        resp.cmd.version.product_code,
        "ICCB",
        sizeof(resp.cmd.version.product_code));
    strncpy(resp.cmd.version.date, __DATE__, sizeof(resp.cmd.version.date));
    strncpy(resp.cmd.version.time, __TIME__, sizeof(resp.cmd.version.time));

    ac_io_emu_response_push(iccb->emu, &resp, 0);
}

static void ac_io_emu_iccb_send_empty(
    struct ac_io_emu_iccb *iccb, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = 0;

    ac_io_emu_response_push(iccb->emu, &resp, 0);
}

static void ac_io_emu_iccb_send_status(
    struct ac_io_emu_iccb *iccb,
    const struct ac_io_message *req,
    uint8_t status)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.status);
    resp.cmd.status = status;

    ac_io_emu_response_push(iccb->emu, &resp, 0);
}

static void ac_io_emu_iccb_send_state(
    struct ac_io_emu_iccb *iccb, const struct ac_io_message *req)
{
    struct ac_io_message resp;
    struct ac_io_iccb_state *body;
    bool sensor;

    /* state update */

    if (!eam_io_poll(iccb->unit_no)) {
        log_warning("Polling eamio failed");
    }

    sensor = eam_io_get_sensor_state(iccb->unit_no);

    if (sensor != iccb->last_sensor) {
        if (sensor) {
            iccb->card_result =
                eam_io_read_card(iccb->unit_no, iccb->uid, sizeof(iccb->uid));

            // fault if sensor says to read but we got no card
            iccb->fault = (iccb->card_result == EAM_IO_CARD_NONE);
        } else {
            iccb->fault = false;
        }
    }

    iccb->last_sensor = sensor;

    if (iccb->fault) {
        memset(iccb->uid, 0, sizeof(iccb->uid));
    }

    /* create response */

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(struct ac_io_iccb_state);

    body = (struct ac_io_iccb_state *) &resp.cmd.raw;

    if (sensor) {
        body->sensor_state = AC_IO_ICCB_SENSOR_CARD;
    } else {
        body->sensor_state = AC_IO_ICCB_SENSOR_NO_CARD;
    }

    if (!iccb->fault) {
        memcpy(body->uid, iccb->uid, sizeof(body->uid));
    }

    body->card_type = 0x30 | (iccb->card_result - 1);
    body->unk2 = 0;
    // this doesn't seem to be an error code. If this is not set to 0x03
    // on slotted readers (only?), the game throws an unknown status error
    body->unk3 = 0x03;
    memset(body->unk4, 0, sizeof(body->unk4));

    ac_io_emu_response_push(iccb->emu, &resp, 0);
}

#include <windows.h>

#include <stdint.h>
#include <string.h>

#include "acio/acio.h"

#include "acioemu/emu.h"

#include "bemanitools/bstio.h"

#include "util/defs.h"

static void kfca_send_version(const struct ac_io_message *req);
static void kfca_report_status(const struct ac_io_message *req, uint8_t status);
static void kfca_report_nil(const struct ac_io_message *req);
static void kfca_poll(const struct ac_io_message *req);

static struct ac_io_emu *kfca_ac_io_emu;

void kfca_init(struct ac_io_emu *emu)
{
    kfca_ac_io_emu = emu;
}

void kfca_dispatch_request(const struct ac_io_message *req)
{
    uint16_t cmd_code;

    cmd_code = ac_io_u16(req->cmd.code);

    switch (cmd_code) {
    case AC_IO_CMD_GET_VERSION:
        log_misc("AC_IO_CMD_GET_VERSION(%d)", req->addr);
        kfca_send_version(req);

        break;

    case AC_IO_CMD_START_UP:
        log_misc("AC_IO_CMD_START_UP(%d)", req->addr);
        kfca_report_status(req, 0x00);

        break;

    case AC_IO_CMD_KFCA_POLL:
        kfca_poll(req);

        break;

    case AC_IO_CMD_KFCA_WATCHDOG:
        log_misc("AC_IO_CMD_KFCA_WATCHDOG(%d)", req->addr);
        kfca_report_status(req, 0x00);

        break;

    case AC_IO_CMD_KFCA_AMP_CONTROL:
        log_misc("AC_IO_CMD_KFCA_AMP_CONTROL(%d)", req->addr);
        kfca_report_nil(req);

        break;

    default:
        log_warning("Unknown ACIO message %04x on KFCA mode, addr=%d",
                cmd_code, req->addr);

        break;
    }
}

static void kfca_send_version(const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.version);
    resp.cmd.version.type = ac_io_u32(AC_IO_NODE_TYPE_KFCA);
    resp.cmd.version.flag = 0x00;
    resp.cmd.version.major = 0x01;
    resp.cmd.version.minor = 0x01;
    resp.cmd.version.revision = 0x00;
    memcpy(resp.cmd.version.product_code, "KFCA",
            sizeof(resp.cmd.version.product_code));
    strncpy(resp.cmd.version.date, __DATE__, sizeof(resp.cmd.version.date));
    strncpy(resp.cmd.version.time, __TIME__, sizeof(resp.cmd.version.time));

    ac_io_emu_response_push(kfca_ac_io_emu, &resp, 0);
}

static void kfca_report_status(const struct ac_io_message *req, uint8_t status)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.status);
    resp.cmd.status = status;

    ac_io_emu_response_push(kfca_ac_io_emu, &resp, 0);
}

static void kfca_report_nil(const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = 0;

    ac_io_emu_response_push(kfca_ac_io_emu, &resp, 0);
}

static void kfca_poll(const struct ac_io_message *req)
{
    struct ac_io_message resp;
    struct ac_io_kfca_poll_in *pin;

    bst_io_read_input();

    pin = &resp.cmd.kfca_poll_in;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(*pin);

    memset(pin, 0, sizeof(*pin));

    pin->gpio_sys = ac_io_u16(bst_io_get_input());

    ac_io_emu_response_push(kfca_ac_io_emu, &resp, 0);
}


#include <windows.h>

#include <stdint.h>
#include <string.h>

#include "acio/acio.h"

#include "acioemu/emu.h"

#include "iface-core/log.h"
#include "iface-io/sdvx.h"

#include "util/defs.h"
#include "util/time.h"

static void kfca_send_version(const struct ac_io_message *req);
static void kfca_report_status(const struct ac_io_message *req, uint8_t status);
static void kfca_report_0128(const struct ac_io_message *req);
static void kfca_poll(const struct ac_io_message *req);
static void kfca_poll_thunk(void *ctx_ptr, struct ac_io_message *resp);

static struct ac_io_emu *kfca_ac_io_emu;

void kfca_init(struct ac_io_emu *emu)
{
    log_assert(emu != NULL);

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
            kfca_report_0128(req);

            break;

        default:
            log_warning(
                "Unknown ACIO message %04x on KFCA mode, addr=%d",
                cmd_code,
                req->addr);

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
    memcpy(
        resp.cmd.version.product_code,
        "KFCA",
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

static void kfca_report_0128(const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = req->cmd.nbytes;
    memcpy(resp.cmd.raw, req->cmd.raw, req->cmd.nbytes);

    ac_io_emu_response_push(kfca_ac_io_emu, &resp, 0);
}

static void kfca_poll(const struct ac_io_message *req)
{
    static uint32_t last_poll = 0;
    uint64_t delay;
    const struct ac_io_kfca_poll_out *pout;
    uintptr_t ctx;
    size_t i;

    /* Handle lighting output immediately */

    pout = &req->cmd.kfca_poll_out;

    bt_io_sdvx_gpio_lights_set(ac_io_u32(pout->gpio));

    for (i = 0; i < lengthof(pout->pwm); i++) {
        bt_io_sdvx_pwm_light_set(i, pout->pwm[i]);
    }

    bt_io_sdvx_output_write();

    /*  SDVX expects only one poll response per frame.

        We use a thunk here to defer polling until right before the response
        is due. */

    delay = time_get_elapsed_ms(last_poll) < 16 ? 16000 : 0;
    last_poll = time_get_counter();
    ctx = req->addr | (req->cmd.code << 8) | (req->cmd.seq_no << 24);

    ac_io_emu_response_push_thunk(
        kfca_ac_io_emu, kfca_poll_thunk, (void *) ctx, delay);
}

static void kfca_poll_thunk(void *ctx_ptr, struct ac_io_message *resp)
{
    struct ac_io_kfca_poll_in *pin;
    uintptr_t ctx;
    uint8_t req_addr;
    uint16_t req_code;
    uint8_t req_seq_no;

    /* Unpack context "pointer" for relevant response information */

    ctx = (uintptr_t) ctx_ptr;
    req_addr = ctx;
    req_code = ctx >> 8;
    req_seq_no = ctx >> 24;

    /* Poll input now and construct an immediate response */

    pin = &resp->cmd.kfca_poll_in;

    resp->addr = req_addr | AC_IO_RESPONSE_FLAG;
    resp->cmd.code = req_code;
    resp->cmd.seq_no = req_seq_no;
    resp->cmd.nbytes = sizeof(*pin);

    bt_io_sdvx_input_read();

    memset(pin, 0, sizeof(*pin));

    pin->adc[0] = bt_io_sdvx_spinner_pos_get(0) << 6;
    pin->adc[1] = bt_io_sdvx_spinner_pos_get(1) << 6;

    pin->gpio_sys |= bt_io_sdvx_input_gpio_sys_get() & 0x3F;

    pin->adc[0] = ac_io_u16(pin->adc[0]);
    pin->adc[1] = ac_io_u16(pin->adc[1]);
    pin->gpio[0] = ac_io_u16(bt_io_sdvx_input_gpio_get(0));
    pin->gpio[1] = ac_io_u16(bt_io_sdvx_input_gpio_get(1));
}

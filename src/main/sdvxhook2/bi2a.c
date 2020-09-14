#define LOG_MODULE "sdvxhook2-bi2a"

#include <windows.h> /* for _BitScanForward */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "acioemu/emu.h"
#include "bio2emu/emu.h"
#include "sdvxhook2/bi2a.h"

#include "bemanitools/sdvxio.h"

static void bio2_emu_bi2a_cmd_send_version(
    struct ac_io_emu *emu, const struct ac_io_message *req);
static void bio2_emu_bi2a_send_state(
    struct ac_io_emu *emu, const struct ac_io_message *req);
static void bio2_emu_bi2a_send_empty(
    struct ac_io_emu *emu, const struct ac_io_message *req);
static void bio2_emu_bi2a_send_status(
    struct ac_io_emu *emu, const struct ac_io_message *req, uint8_t status);
static void
kfca_amp_control(struct ac_io_emu *emu, const struct ac_io_message *req);

static bool poll_delay;

static bool force_headphones_on;

void bio2_emu_bi2a_init(
    struct bio2emu_port *bio2_emu,
    bool disable_poll_limiter,
    bool force_headphones)
{
    bio2emu_port_init(bio2_emu);

    poll_delay = !disable_poll_limiter;

    if (!poll_delay) {
        log_warning("bio2_emu_bi2a_init: poll_delay has been disabled");
    }

    force_headphones_on = force_headphones;
}

void bio2_emu_bi2a_dispatch_request(
    struct bio2emu_port *bio2port, const struct ac_io_message *req)
{
    uint16_t cmd_code;

    cmd_code = ac_io_u16(req->cmd.code);

    switch (cmd_code) {
        case AC_IO_CMD_KFCA_WATCHDOG:
            log_misc("BIO2_BI2A_CMD_WATCHDOG(%d)", req->addr);
            bio2_emu_bi2a_send_status(&bio2port->acio, req, 0x00);
            break;

        case AC_IO_CMD_KFCA_POLL:
            // log_misc("BIO2_BI2A_CMD_POLL");
            bio2_emu_bi2a_send_state(&bio2port->acio, req);
            break;

        case AC_IO_CMD_GET_VERSION:
            log_misc("BIO2_CMD_GET_VERSION(%d)", req->addr);
            bio2_emu_bi2a_cmd_send_version(&bio2port->acio, req);
            break;

        case AC_IO_CMD_CLEAR:
            log_misc("BIO2_BI2A_CMD_CLEAR(%d)", req->addr);
            bio2_emu_bi2a_send_status(&bio2port->acio, req, 0x00);
            break;

        case AC_IO_CMD_START_UP:
            log_misc("BIO2_BI2A_CMD_START_UP(%d)", req->addr);
            bio2_emu_bi2a_send_status(&bio2port->acio, req, 0x00);
            break;

        case AC_IO_CMD_KEEPALIVE:
            log_misc("BIO2_BI2A_CMD_KEEPALIVE(%d)", req->addr);
            bio2_emu_bi2a_send_empty(&bio2port->acio, req);
            break;

        case AC_IO_CMD_KFCA_AMP_CONTROL:
            log_misc("BIO2_BI2A_CMD_AMP_CONTROL(%d)", req->addr);
            kfca_amp_control(&bio2port->acio, req);
            break;

        default:
            log_warning(
                "Unknown BIO2 message %04x on BI2A node, addr=%d",
                cmd_code,
                req->addr);
            break;
    }
}

static void
kfca_amp_control(struct ac_io_emu *emu, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = req->cmd.nbytes;
    memcpy(resp.cmd.raw, req->cmd.raw, req->cmd.nbytes);

    // so SDVX5 sets the amps to 0 aka ALL MAX and uses Windows instead
    // let's leave it to the individual sdvxio to set on init instead
    // this way if any sdvxio's want to use Windows APIs for SDVX1-4 comapt.
    // they can do so without doubly affecting the volume

    // sdvx_io_set_amp_volume(0, 0, 0, 0);

    ac_io_emu_response_push(emu, &resp, 0);
}

static void bio2_emu_bi2a_cmd_send_version(
    struct ac_io_emu *emu, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.version);
    resp.cmd.version.type = ac_io_u32(AC_IO_NODE_TYPE_BI2A);
    resp.cmd.version.flag = 0x00;
    resp.cmd.version.major = 0x01;
    resp.cmd.version.minor = 0x02;
    resp.cmd.version.revision = 0x09;
    memcpy(
        resp.cmd.version.product_code,
        "BI2A",
        sizeof(resp.cmd.version.product_code));
    strncpy(resp.cmd.version.date, __DATE__, sizeof(resp.cmd.version.date));
    strncpy(resp.cmd.version.time, __TIME__, sizeof(resp.cmd.version.time));

    ac_io_emu_response_push(emu, &resp, 0);
}

static void
bio2_emu_bi2a_send_empty(struct ac_io_emu *emu, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = 0;

    ac_io_emu_response_push(emu, &resp, 0);
}

static void bio2_emu_bi2a_send_status(
    struct ac_io_emu *emu, const struct ac_io_message *req, uint8_t status)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.status);
    resp.cmd.status = status;

    ac_io_emu_response_push(emu, &resp, 0);
}

static uint8_t check_pin(uint16_t value, uint8_t pin)
{
    return (value >> pin) & 1;
}

static uint32_t assign_light(uint32_t shift, uint32_t value)
{
    if (!value) {
        return 0;
    } else {
        return 1 << shift;
    }
}

static void
bio2_emu_bi2a_send_state(struct ac_io_emu *emu, const struct ac_io_message *req)
{
    struct ac_io_message resp;
    struct bi2a_sdvx_state_in *pin;
    struct bi2a_sdvx_state_out *pout;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(*pin);

    pout = (struct bi2a_sdvx_state_out *) &req->cmd.raw;
    pin = (struct bi2a_sdvx_state_in *) &resp.cmd.raw;
    memset(pin, 0, sizeof(*pin));

    uint32_t gpio = 0;
    gpio |= assign_light(0x0C, pout->gpio[0]);
    gpio |= assign_light(0x0D, pout->gpio[1]);
    gpio |= assign_light(0x0E, pout->gpio[2]);
    gpio |= assign_light(0x0F, pout->gpio[3]);
    gpio |= assign_light(0x00, pout->gpio[4]);
    gpio |= assign_light(0x01, pout->gpio[5]);
    gpio |= assign_light(0x02, pout->gpio[6]);

    sdvx_io_set_gpio_lights(gpio);

    sdvx_io_set_pwm_light(0x0, pout->wingUpper[0]);
    sdvx_io_set_pwm_light(0x1, pout->wingUpper[1]);
    sdvx_io_set_pwm_light(0x2, pout->wingUpper[2]);
    sdvx_io_set_pwm_light(0x3, pout->wingUpper[0]);
    sdvx_io_set_pwm_light(0x4, pout->wingUpper[1]);
    sdvx_io_set_pwm_light(0x5, pout->wingUpper[2]);

    sdvx_io_set_pwm_light(0x6, pout->wingLower[0]);
    sdvx_io_set_pwm_light(0x7, pout->wingLower[1]);
    sdvx_io_set_pwm_light(0x8, pout->wingLower[2]);
    sdvx_io_set_pwm_light(0x9, pout->wingLower[0]);
    sdvx_io_set_pwm_light(0xA, pout->wingLower[1]);
    sdvx_io_set_pwm_light(0xB, pout->wingLower[2]);

    sdvx_io_set_pwm_light(0xC, pout->woof_r);
    sdvx_io_set_pwm_light(0xD, pout->woof_g);
    sdvx_io_set_pwm_light(0xE, pout->woof_b);

    sdvx_io_set_pwm_light(0xF, pout->controller[0]);
    sdvx_io_set_pwm_light(0x10, pout->controller[1]);
    sdvx_io_set_pwm_light(0x11, pout->controller[2]);

    sdvx_io_write_output();

    sdvx_io_read_input();
    uint8_t sys = sdvx_io_get_input_gpio_sys();
    uint16_t gpio0 = sdvx_io_get_input_gpio(0);
    uint16_t gpio1 = sdvx_io_get_input_gpio(1);

    // TODO: Make a counter or smth to counteract the accuracy lost in *4
    pin->analogs[0].a_val = sdvx_io_get_spinner_pos(0);
    pin->analogs[1].a_val = sdvx_io_get_spinner_pos(1);

    pin->analogs[0].a_coin = check_pin(sys, SDVX_IO_IN_GPIO_SYS_COIN);
    pin->analogs[0].a_test = check_pin(sys, SDVX_IO_IN_GPIO_SYS_TEST);
    pin->analogs[0].a_service = check_pin(sys, SDVX_IO_IN_GPIO_SYS_SERVICE) ||
        check_pin(sys, SDVX_IO_IN_GPIO_SYS_COIN);
    pin->raw[0] = ac_io_u16(pin->raw[0]);
    pin->raw[1] = ac_io_u16(pin->raw[1]);

    pin->buttons_1.b_start = check_pin(gpio0, SDVX_IO_IN_GPIO_0_START);
    pin->buttons_1.b_headphone = check_pin(gpio0, SDVX_IO_IN_GPIO_0_HEADPHONE);
    if (force_headphones_on) {
        pin->buttons_1.b_headphone = 1;
    }
    pin->buttons_1.b_a = check_pin(gpio0, SDVX_IO_IN_GPIO_0_A);
    pin->buttons_1.b_b = check_pin(gpio0, SDVX_IO_IN_GPIO_0_B);
    pin->buttons_1.b_c = check_pin(gpio0, SDVX_IO_IN_GPIO_0_C);
    pin->buttons_1.b_d = check_pin(gpio1, SDVX_IO_IN_GPIO_1_D);
    pin->buttons_1.b_fxl = check_pin(gpio1, SDVX_IO_IN_GPIO_1_FX_L);
    pin->buttons_2.b_fxr = check_pin(gpio1, SDVX_IO_IN_GPIO_1_FX_R);

    ac_io_emu_response_push(emu, &resp, 0);
}

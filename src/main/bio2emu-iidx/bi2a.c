#define LOG_MODULE "bio2emu-iidx"

#include "bio2emu-iidx/bi2a.h"

#include <windows.h> /* for _BitScanForward */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "acioemu/emu.h"

#include "bemanitools/iidxio.h"

static int get_default_slider_valid(size_t idx);
static void bio2_emu_bi2a_cmd_send_version(
    struct ac_io_emu *emu, const struct ac_io_message *req);
static void bio2_emu_bi2a_send_state(
    struct ac_io_emu *emu, const struct ac_io_message *req);
static void bio2_emu_bi2a_send_empty(
    struct ac_io_emu *emu, const struct ac_io_message *req);
static void bio2_emu_bi2a_send_status(
    struct ac_io_emu *emu, const struct ac_io_message *req, uint8_t status);

static int default_sliders[5];
static bool poll_delay;

int get_default_slider_valid(size_t idx)
{
    if (default_sliders[idx] >= 0 && default_sliders[idx] <= 15) {
        return 1;
    } else {
        return 0;
    }
}

void bio2_emu_bi2a_init(
    struct bio2emu_port *bio2_emu, bool disable_poll_limiter)
{
    bio2emu_port_init(bio2_emu);

    poll_delay = !disable_poll_limiter;
    if (!poll_delay) {
        log_warning("bio2_emu_bi2a_init: poll_delay has been disabled");
    }

    for (size_t i = 0; i < 5; ++i) {
        default_sliders[i] = -1;
    }

    FILE *f = fopen("vefx.txt", "r");
    if (f) {
        fscanf(
            f,
            "%d %d %d %d %d",
            &default_sliders[0],
            &default_sliders[1],
            &default_sliders[2],
            &default_sliders[3],
            &default_sliders[4]);
        fclose(f);
    }
}

void bio2_emu_bi2a_dispatch_request(
    struct bio2emu_port *bio2port, const struct ac_io_message *req)
{
    uint16_t cmd_code;

    cmd_code = ac_io_u16(req->cmd.code);

    switch (cmd_code) {
        case BIO2_BI2A_CMD_UNK_0100:
        case BIO2_BI2A_CMD_UNK_0120:
            log_misc("BIO2_BI2A_CMD_UNK_%04X(%d)", cmd_code, req->addr);
            bio2_emu_bi2a_send_status(&bio2port->acio, req, 0x00);
            break;

        case BIO2_BI2A_CMD_POLL:
            // log_misc("BIO2_BI2A_CMD_POLL");
            bio2_emu_bi2a_send_state(&bio2port->acio, req);
            break;

        case AC_IO_CMD_GET_VERSION:
            log_misc("BIO2_CMD_GET_VERSION(%d)", req->addr);
            bio2_emu_bi2a_cmd_send_version(&bio2port->acio, req);
            break;

        case AC_IO_CMD_START_UP:
            log_misc("BIO2_CMD_START_UP(%d)", req->addr);
            bio2_emu_bi2a_send_status(&bio2port->acio, req, 0x00);
            break;

        case AC_IO_CMD_KEEPALIVE:
            log_misc("BIO2_CMD_KEEPALIVE(%d)", req->addr);
            bio2_emu_bi2a_send_empty(&bio2port->acio, req);
            break;

        default:
            log_warning(
                "Unknown BIO2 message %04x on BI2A node, addr=%d",
                cmd_code,
                req->addr);
            break;
    }
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
    resp.cmd.version.major = 0x04;
    resp.cmd.version.minor = 0x00;
    resp.cmd.version.revision = 0x04;
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

static void
bio2_emu_bi2a_send_state(struct ac_io_emu *emu, const struct ac_io_message *req)
{
    struct ac_io_message resp;
    struct bi2a_iidx_state_out *body;
    struct bi2a_iidx_state_in *req_bi2a;

    uint8_t input_sys = 0;
    uint8_t input_panel = 0;
    uint16_t input_keys = 0;

    struct {
        uint8_t panel_lights;
        uint16_t deck_lights;
        uint8_t top_lamps;
    } packed_lights;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(struct bi2a_iidx_state_out);
    req_bi2a = (struct bi2a_iidx_state_in *) req->cmd.raw;

    packed_lights.panel_lights = 0;
    for (size_t i = 0; i < 4; ++i) {
        if (req_bi2a->PANEL[i].l_state) {
            packed_lights.panel_lights |= (1 << i);
        }
    }
    packed_lights.deck_lights = 0;
    for (size_t i = 0; i < 14; ++i) {
        if (req_bi2a->DECKSW[i].l_state) {
            packed_lights.deck_lights |= (1 << i);
        }
    }
    packed_lights.top_lamps = 0;
    for (size_t i = 0; i < 4; ++i) {
        if (req_bi2a->SPOTLIGHT1[i].l_state) {
            packed_lights.top_lamps |= (1 << i);
        }
    }
    for (size_t i = 0; i < 4; ++i) {
        if (req_bi2a->SPOTLIGHT2[i].l_state) {
            packed_lights.top_lamps |= (1 << (i + 4));
        }
    }

    iidx_io_ep1_set_deck_lights(packed_lights.deck_lights);
    iidx_io_ep1_set_panel_lights(packed_lights.panel_lights);
    iidx_io_ep1_set_top_lamps(packed_lights.top_lamps);
    iidx_io_ep1_set_top_neons(req_bi2a->NEONLAMP.l_state);

    if (!iidx_io_ep1_send()) {
        log_warning("BIO2: iidx_io_ep1_send error");
        return bio2_emu_bi2a_send_status(emu, req, 0);
    }

    if (!iidx_io_ep3_write_16seg((const char *) req_bi2a->SEG16)) {
        log_warning("BIO2: iidx_io_ep3_write_16seg error");
        return bio2_emu_bi2a_send_status(emu, req, 0);
    }

    body = (struct bi2a_iidx_state_out *) &resp.cmd.raw;
    memset(body, 0, sizeof(struct bi2a_iidx_state_out));

    // IIDX25 polls really really fast, this limits it to 1000Hz
    if (poll_delay) {
        Sleep(1);
    }

    if (!iidx_io_ep2_recv()) {
        log_warning("BIO2: iidx_io_ep2_recv error");
        return bio2_emu_bi2a_send_status(emu, req, 0);
    }

    body->TURNTABLE1 = iidx_io_ep2_get_turntable(0);
    body->TURNTABLE2 = iidx_io_ep2_get_turntable(1);

    body->SLIDER1.s_val = get_default_slider_valid(0) ?
        default_sliders[0] :
        iidx_io_ep2_get_slider(0);
    body->SLIDER2.s_val = get_default_slider_valid(1) ?
        default_sliders[1] :
        iidx_io_ep2_get_slider(1);
    body->SLIDER3.s_val = get_default_slider_valid(2) ?
        default_sliders[2] :
        iidx_io_ep2_get_slider(2);
    body->SLIDER4.s_val = get_default_slider_valid(3) ?
        default_sliders[3] :
        iidx_io_ep2_get_slider(3);
    body->SLIDER5.s_val = get_default_slider_valid(4) ?
        default_sliders[4] :
        iidx_io_ep2_get_slider(4);

    input_keys = iidx_io_ep2_get_keys();
    input_sys = iidx_io_ep2_get_sys();
    input_panel = iidx_io_ep2_get_panel();
    body->P1SW1.b_val = (input_keys >> IIDX_IO_KEY_P1_1) & 1;
    body->P1SW2.b_val = (input_keys >> IIDX_IO_KEY_P1_2) & 1;
    body->P1SW3.b_val = (input_keys >> IIDX_IO_KEY_P1_3) & 1;
    body->P1SW4.b_val = (input_keys >> IIDX_IO_KEY_P1_4) & 1;
    body->P1SW5.b_val = (input_keys >> IIDX_IO_KEY_P1_5) & 1;
    body->P1SW6.b_val = (input_keys >> IIDX_IO_KEY_P1_6) & 1;
    body->P1SW7.b_val = (input_keys >> IIDX_IO_KEY_P1_7) & 1;
    body->P2SW1.b_val = (input_keys >> IIDX_IO_KEY_P2_1) & 1;
    body->P2SW2.b_val = (input_keys >> IIDX_IO_KEY_P2_2) & 1;
    body->P2SW3.b_val = (input_keys >> IIDX_IO_KEY_P2_3) & 1;
    body->P2SW4.b_val = (input_keys >> IIDX_IO_KEY_P2_4) & 1;
    body->P2SW5.b_val = (input_keys >> IIDX_IO_KEY_P2_5) & 1;
    body->P2SW6.b_val = (input_keys >> IIDX_IO_KEY_P2_6) & 1;
    body->P2SW7.b_val = (input_keys >> IIDX_IO_KEY_P2_7) & 1;

    body->PANEL.y_start1 = (input_panel >> IIDX_IO_PANEL_P1_START) & 1;
    body->PANEL.y_start2 = (input_panel >> IIDX_IO_PANEL_P2_START) & 1;
    body->PANEL.y_vefx = (input_panel >> IIDX_IO_PANEL_VEFX) & 1;
    body->PANEL.y_effect = (input_panel >> IIDX_IO_PANEL_EFFECT) & 1;

    body->SYSTEM.v_test = (input_sys >> IIDX_IO_SYS_TEST) & 1;
    body->SYSTEM.v_service = (input_sys >> IIDX_IO_SYS_SERVICE) & 1;
    body->SYSTEM.v_coin = (input_sys >> IIDX_IO_SYS_COIN) & 1;

    ac_io_emu_response_push(emu, &resp, 0);
}

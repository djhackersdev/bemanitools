#define LOG_MODULE "acioemu-icca"

#include "acioemu/icca.h"

#include <windows.h> /* for _BitScanForward */

#include <stdint.h>
#include <string.h>

#include "acio/icca.h"

#include "acioemu/emu.h"
#include "acioemu/icca.h"

#include "bemanitools/eamio.h"

#include "util/time.h"

enum ac_io_icca_subcmd {
    AC_IO_ICCA_SUBCMD_CARD_SLOT_CLOSE = 0x00,
    AC_IO_ICCA_SUBCMD_CARD_SLOT_OPEN = 0x11,
    AC_IO_ICCA_SUBCMD_CARD_SLOT_EJECT = 0x12,
};

enum ac_io_icca_flag {
    AC_IO_ICCA_FLAG_FRONT_SENSOR = 0x10,
    AC_IO_ICCA_FLAG_REAR_SENSOR = 0x20,
    AC_IO_ICCA_FLAG_SOLENOID = 0x40
};

enum ac_io_icca_status_code {
    AC_IO_ICCA_STATUS_FAULT = 0x00,
    AC_IO_ICCA_STATUS_IDLE = 0x01,
    AC_IO_ICCA_STATUS_GOT_UID = 0x02,
    AC_IO_ICCA_STATUS_IDLE_NEW = 0x04
};

static void ac_io_emu_icca_cmd_send_version(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req);

static void ac_io_emu_icca_send_state(
    struct ac_io_emu_icca *icca,
    const struct ac_io_message *req,
    uint64_t delay_us);

static void ac_io_emu_icca_send_empty(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req);

static void ac_io_emu_icca_send_status(
    struct ac_io_emu_icca *icca,
    const struct ac_io_message *req,
    uint8_t status);

void ac_io_emu_icca_init(
    struct ac_io_emu_icca *icca, struct ac_io_emu *emu, uint8_t unit_no)
{
    memset(icca, 0, sizeof(*icca));
    icca->emu = emu;
    icca->unit_no = unit_no;
    // queue must be started
    icca->fault = true;
}

void ac_io_emu_icca_dispatch_request(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req)
{
    uint16_t cmd_code;
    uint64_t delay_us;

    cmd_code = ac_io_u16(req->cmd.code);

    switch (cmd_code) {
        case AC_IO_CMD_GET_VERSION:
            log_misc("AC_IO_CMD_GET_VERSION(%d)", req->addr);
            ac_io_emu_icca_cmd_send_version(icca, req);

            break;

        case AC_IO_CMD_START_UP:
            log_misc("AC_IO_CMD_START_UP(%d)", req->addr);
            icca->detected_new_reader = false;
            ac_io_emu_icca_send_status(icca, req, 0x00);

            break;

        case AC_IO_CMD_CLEAR:
            log_misc("AC_IO_ICCA_CMD_CLEAR(%d)", req->addr);
            ac_io_emu_icca_send_status(icca, req, 0x00);

            break;

        case AC_IO_CMD_KEEPALIVE:
            ac_io_emu_icca_send_empty(icca, req);

            break;

        case AC_IO_ICCA_CMD_QUEUE_LOOP_START:
            log_misc("AC_IO_CMD_QUEUE_LOOP_START(%d)", req->addr);
            // queue started, reset error state
            icca->fault = false;
            ac_io_emu_icca_send_status(icca, req, 0x00);
            icca->polling_started = true;

            break;

        case AC_IO_CMD_UNKN_00FF:
            log_misc("AC_IO_CMD_UNKN_00FF(%d)", req->addr);
            ac_io_emu_icca_send_status(icca, req, 0x00);

            break;

        case AC_IO_ICCA_CMD_UNKN_0120:
            log_misc("AC_IO_ICCA_CMD_UNKN_0120(%d)", req->addr);
            ac_io_emu_icca_send_status(icca, req, 0x00);

            break;

        case AC_IO_ICCA_CMD_BEGIN_KEYPAD:
            log_misc("AC_IO_ICCA_CMD_BEGIN_KEYPAD(%d)", req->addr);
            ac_io_emu_icca_send_status(icca, req, 0x00);
            icca->keypad_started = true;

            break;

        case AC_IO_ICCA_CMD_ENGAGE:
            ac_io_emu_icca_send_state(icca, req, 0);

            break;

        case AC_IO_ICCA_CMD_SET_SLOT_STATE: {
            struct ac_io_icca_misc *misc =
                (struct ac_io_icca_misc *) &req->cmd.raw;
            uint8_t cmd;

            switch (misc->subcmd) {
                case AC_IO_ICCA_SUBCMD_CARD_SLOT_CLOSE:
                    cmd = EAM_IO_CARD_SLOT_CMD_CLOSE;
                    break;

                case AC_IO_ICCA_SUBCMD_CARD_SLOT_OPEN:
                    cmd = EAM_IO_CARD_SLOT_CMD_OPEN;
                    break;

                case AC_IO_ICCA_SUBCMD_CARD_SLOT_EJECT:
                    cmd = EAM_IO_CARD_SLOT_CMD_EJECT;
                    icca->engaged = false;
                    break;

                case 3:
                    cmd = EAM_IO_CARD_SLOT_CMD_READ;
                    break;

                default:
                    cmd = 0xFF;
                    log_warning(
                        "Unhandled slot command %X, node %d",
                        misc->subcmd,
                        icca->unit_no);
                    break;
            }

            if (cmd != 0xFF) {
                if (!eam_io_card_slot_cmd(icca->unit_no, cmd)) {
                    log_warning(
                        "Eamio failed to handle slot cmd %d for node %d",
                        cmd,
                        icca->unit_no);
                }
            }

            /* response with current slot state */
            ac_io_emu_icca_send_status(icca, req, misc->subcmd);

            break;
        }

        case AC_IO_ICCA_CMD_POLL:
            delay_us = time_get_elapsed_us(
                time_get_counter() - icca->time_counter_last_poll);

            /* emulating delay implemented by hardware. do not delay messages
             * that exceed a certain threshold. */
            if (delay_us > 16000) {
                delay_us = 0;
            }

            icca->time_counter_last_poll = time_get_counter();
            ac_io_emu_icca_send_state(icca, req, delay_us);

            break;

        case AC_IO_ICCA_CMD_POLL_FELICA:
            icca->detected_new_reader = true;
            ac_io_emu_icca_send_status(icca, req, 0x01);

            break;

        default:
            log_warning(
                "Unknown ACIO message %04x on ICCA node, addr=%d",
                cmd_code,
                req->addr);

            break;
    }
}

static void ac_io_emu_icca_cmd_send_version(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.version);
    resp.cmd.version.type = ac_io_u32(AC_IO_NODE_TYPE_ICCA);
    resp.cmd.version.flag = 0x00;
    resp.cmd.version.major = 0x01;
    resp.cmd.version.minor = 0x06;
    resp.cmd.version.revision = 0x00;
    memcpy(
        resp.cmd.version.product_code,
        "ICCA",
        sizeof(resp.cmd.version.product_code));
    strncpy(resp.cmd.version.date, __DATE__, sizeof(resp.cmd.version.date));
    strncpy(resp.cmd.version.time, __TIME__, sizeof(resp.cmd.version.time));

    ac_io_emu_response_push(icca->emu, &resp, 0);
}

static void ac_io_emu_icca_send_empty(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = 0;

    ac_io_emu_response_push(icca->emu, &resp, 0);
}

static void ac_io_emu_icca_send_status(
    struct ac_io_emu_icca *icca,
    const struct ac_io_message *req,
    uint8_t status)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(resp.cmd.status);
    resp.cmd.status = status;

    ac_io_emu_response_push(icca->emu, &resp, 0);
}

static void ac_io_emu_icca_send_state(
    struct ac_io_emu_icca *icca,
    const struct ac_io_message *req,
    uint64_t delay_us)
{
    struct ac_io_message resp;
    struct ac_io_icca_state *body;
    unsigned long bit;
    uint8_t event;
    uint16_t keypad;
    uint16_t keypad_rise;
    uint8_t sensor_state;
    bool card_full_insert;

    if (!eam_io_poll(icca->unit_no)) {
        log_warning("Polling eamio failed");
    }

    memset(&resp, 0, sizeof(resp));

    keypad = eam_io_get_keypad_state(icca->unit_no);
    sensor_state = eam_io_get_sensor_state(icca->unit_no);

    keypad_rise = keypad & (icca->last_keypad ^ keypad);
    card_full_insert = sensor_state & (1 << EAM_IO_SENSOR_FRONT) &&
        sensor_state & (1 << EAM_IO_SENSOR_BACK);

    if (sensor_state != icca->last_sensor) {
        if (card_full_insert) {
            if (!eam_io_card_slot_cmd(
                    icca->unit_no, EAM_IO_CARD_SLOT_CMD_READ)) {
                log_warning(
                    "EAM_IO_CARD_SLOT_CMD_READ to unit %d failed",
                    icca->unit_no);
            }

            icca->card_result =
                eam_io_read_card(icca->unit_no, icca->uid, sizeof(icca->uid));

            // fault if sensor says to read but we got no card
            icca->fault = (icca->card_result == EAM_IO_CARD_NONE);
        } else {
            icca->fault = false;
        }
    }

    icca->last_sensor = sensor_state;
    icca->last_keypad = keypad;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = sizeof(struct ac_io_icca_state);

    body = (struct ac_io_icca_state *) &resp.cmd.raw;

    if (icca->fault) {
        body->status_code = AC_IO_ICCA_STATUS_FAULT;
    } else if (card_full_insert) {
        body->status_code = AC_IO_ICCA_STATUS_GOT_UID;
    } else {
        if (icca->detected_new_reader) {
            body->status_code = AC_IO_ICCA_STATUS_IDLE_NEW;
        } else {
            body->status_code = AC_IO_ICCA_STATUS_IDLE;
        }
    }

    body->sensor_state = 0;

    if (sensor_state & (1 << EAM_IO_SENSOR_FRONT)) {
        body->sensor_state |= AC_IO_ICCA_FLAG_FRONT_SENSOR;
    }

    if (sensor_state & (1 << EAM_IO_SENSOR_BACK)) {
        body->sensor_state |= AC_IO_ICCA_FLAG_REAR_SENSOR;
    }

    if (icca->engaged) {
        body->sensor_state |= AC_IO_ICCA_FLAG_SOLENOID;
    }

    memcpy(body->uid, icca->uid, sizeof(body->uid));
    body->card_type = 0;
    if (body->status_code == AC_IO_ICCA_STATUS_GOT_UID) {
        if (icca->detected_new_reader) {
            // sensor_state actually refers to cardtype for wavepass readers
            // EAM_IO_CARD_ISO15696 = 1 -> 0
            // EAM_IO_CARD_FELICA = 2 -> 1
            body->card_type = icca->card_result - 1;
            body->sensor_state = icca->card_result - 1;
        }
    }

    if (keypad_rise) {
        if (icca->key_events[0]) {
            event = (icca->key_events[0] + 0x10) & 0xF0;
        } else {
            event = 0x00;
        }

        _BitScanForward(&bit, keypad_rise);
        event |= 0x80 | bit;

        icca->key_events[1] = icca->key_events[0];
        icca->key_events[0] = event;
    }

    // this doesn't seem to be an error code. If this is not set to 0x03
    // on slotted readers (only?), the game throws an unknown status error
    // and on SDVX4 it hangs on the thank you screen
    if (icca->keypad_started || icca->detected_new_reader) {
        body->keypad_started = 0x03;
    } else {
        body->keypad_started = 0x00;
    }
    body->key_events[0] = icca->key_events[0];
    body->key_events[1] = icca->key_events[1];
    body->key_state = ac_io_u16(keypad);

    // replace status code if polling hasn't started
    // this fixes SDVX IC CARD boot errors
    if (!icca->polling_started) {
        body->status_code = AC_IO_ICCA_STATUS_FAULT;
    }

    ac_io_emu_response_push(icca->emu, &resp, delay_us);
}

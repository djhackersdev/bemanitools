#define LOG_MODULE "acioemu-icca"

#include "acioemu/icca.h"

#include <windows.h> /* for _BitScanForward */

#include <stdint.h>
#include <string.h>

#include "acio/icca.h"

#include "acioemu/emu.h"
#include "acioemu/icca.h"

#include "iface-io/eam.h"

#include "util/crc.h"
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

static void ac_io_emu_icca_cmd_send_version(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req);

static void ac_io_emu_icca_send_state(
    struct ac_io_emu_icca *icca,
    const struct ac_io_message *req,
    uint64_t delay_us,
    bool encrypted);

static void ac_io_emu_icca_send_empty(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req);

static void ac_io_emu_icca_send_status(
    struct ac_io_emu_icca *icca,
    const struct ac_io_message *req,
    uint8_t status);

static void ac_io_emu_icca_cipher(
    struct ac_io_emu_icca *icca, uint8_t *data, size_t length);

static void ac_io_emu_icca_cipher_set_key(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req);

void ac_io_emu_icca_init(
    struct ac_io_emu_icca *icca, struct ac_io_emu *emu, uint8_t unit_no)
{
    memset(icca, 0, sizeof(*icca));
    icca->emu = emu;
    icca->unit_no = unit_no;

    // queue must be started
    icca->fault = true;

    // default to 1.6.0
    icca->version = v160;
    // default, most common code
    ac_io_emu_icca_set_product_code(icca, AC_IO_EMU_PROD_CODE_ICCA);
}

void ac_io_emu_icca_set_version(
    struct ac_io_emu_icca *icca, enum ac_io_emu_icca_version version)
{
    icca->version = version;
}

void ac_io_emu_icca_set_product_code(
    struct ac_io_emu_icca *icca, const char product_code[4])
{
    memcpy(icca->product_code, product_code, sizeof(icca->product_code));
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

        case AC_IO_ICCA_CMD_UNKN_0116:
        case AC_IO_ICCA_CMD_UNKN_0120:
            log_misc("AC_IO_ICCA_CMD_UNK_%04X(%d)", cmd_code, req->addr);
            ac_io_emu_icca_send_status(icca, req, 0x00);

            break;

        case AC_IO_ICCA_CMD_DEVICE_CONTROL:
            log_misc("AC_IO_ICCA_CMD_DEVICE_CONTROL(%d)", req->addr);
            ac_io_emu_icca_send_status(icca, req, 0x00);
            icca->keypad_started = true;

            break;

        case AC_IO_ICCA_CMD_ENGAGE:
            ac_io_emu_icca_send_state(icca, req, 0, false);

            break;

        case AC_IO_ICCA_CMD_SET_SLOT_STATE: {
            if (icca->version == v150) {
                ac_io_emu_icca_send_state(icca, req, 0, false);
            } else {
                struct ac_io_icca_misc *misc =
                    (struct ac_io_icca_misc *) &req->cmd.raw;
                uint8_t cmd;

                switch (misc->subcmd) {
                    case AC_IO_ICCA_SUBCMD_CARD_SLOT_CLOSE:
                        cmd = BT_IO_EAM_CARD_SLOT_CMD_CLOSE;
                        break;

                    case AC_IO_ICCA_SUBCMD_CARD_SLOT_OPEN:
                        cmd = BT_IO_EAM_CARD_SLOT_CMD_OPEN;
                        break;

                    case AC_IO_ICCA_SUBCMD_CARD_SLOT_EJECT:
                        cmd = BT_IO_EAM_CARD_SLOT_CMD_EJECT;
                        icca->engaged = false;
                        break;

                    case 3:
                        cmd = BT_IO_EAM_CARD_SLOT_CMD_READ;
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
                    if (!bt_io_eam_card_slot_cmd_send(icca->unit_no, cmd)) {
                        log_warning(
                            "Eamio failed to handle slot cmd %d for node %d",
                            cmd,
                            icca->unit_no);
                    }
                }

                /* response with current slot state */
                ac_io_emu_icca_send_status(icca, req, misc->subcmd);
            }

            break;
        }

        case AC_IO_ICCA_CMD_POLL_ENCRYPTED:
        case AC_IO_ICCA_CMD_POLL:
            delay_us = time_get_elapsed_us(
                time_get_counter() - icca->time_counter_last_poll);

            /* emulating delay implemented by hardware. do not delay messages
             * that exceed a certain threshold. */
            if (delay_us > 16000) {
                delay_us = 0;
            }

            icca->time_counter_last_poll = time_get_counter();
            bool encrypted = cmd_code == AC_IO_ICCA_CMD_POLL_ENCRYPTED;
            ac_io_emu_icca_send_state(icca, req, delay_us, encrypted);

            break;

        case AC_IO_ICCA_CMD_POLL_FELICA:
            if (icca->version == v150) {
                ac_io_emu_icca_send_state(icca, req, 0, false);
            } else if (icca->version == v170) {
                ac_io_emu_icca_send_empty(icca, req);
                icca->detected_new_reader = true;
            } else {
                ac_io_emu_icca_send_status(icca, req, 0x01);
                icca->detected_new_reader = true;
            }

            break;

        case AC_IO_ICCA_CMD_KEY_EXCHANGE:
            icca->detected_new_reader = true;
            log_misc("AC_IO_ICCA_CMD_KEY_EXCHANGE(%d)", req->addr);
            ac_io_emu_icca_cipher_set_key(icca, req);

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

    if (icca->version == v150) {
        resp.cmd.version.minor = 0x05;
    } else if (icca->version == v160) {
        resp.cmd.version.minor = 0x06;
    } else if (icca->version == v170) {
        resp.cmd.version.minor = 0x07;
    } else {
        // probably log invalid version here
        log_warning(
            "Unknown ICCA version: %d emulation requested", icca->version);
    }

    resp.cmd.version.revision = 0x00;

    memcpy(
        resp.cmd.version.product_code,
        icca->product_code,
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
    uint64_t delay_us,
    bool encrypted)
{
    struct ac_io_message resp;
    struct ac_io_icca_state *body;
    unsigned long bit;
    uint8_t event;
    uint16_t keypad;
    uint16_t keypad_rise;
    uint8_t sensor_state;
    bool card_full_insert;

    if (!bt_io_eam_poll(icca->unit_no)) {
        log_warning("Polling eamio failed");
    }

    memset(&resp, 0, sizeof(resp));

    keypad = bt_io_eam_keypad_state_get(icca->unit_no);
    sensor_state = bt_io_eam_sensor_state_get(icca->unit_no);

    keypad_rise = keypad & (icca->last_keypad ^ keypad);
    card_full_insert = sensor_state & (1 << BT_IO_EAM_SENSOR_STATE_FRONT) &&
        sensor_state & (1 << BT_IO_EAM_SENSOR_STATE_BACK);

    if (sensor_state != icca->last_sensor) {
        if (card_full_insert) {
            if (!bt_io_eam_card_slot_cmd_send(
                    icca->unit_no, BT_IO_EAM_CARD_SLOT_CMD_READ)) {
                log_warning(
                    "BT_IO_EAM_CARD_SLOT_CMD_READ to unit %d failed",
                    icca->unit_no);
            }

            icca->card_result = bt_io_eam_card_read(
                icca->unit_no, icca->uid, sizeof(icca->uid));

            // fault if sensor says to read but we got no card
            icca->fault =
                (icca->card_result == BT_IO_EAM_READ_CARD_RESULT_NONE);
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
        if (icca->detected_new_reader && icca->version == v170) {
            // else we get a power failure?
            body->status_code = 0;
        } else if (icca->detected_new_reader) {
            body->status_code = AC_IO_ICCA_STATUS_IDLE_NEW;
        } else {
            body->status_code = AC_IO_ICCA_STATUS_IDLE;
        }
    }

    body->sensor_state = 0;

    if (sensor_state & (1 << BT_IO_EAM_SENSOR_STATE_FRONT)) {
        body->sensor_state |= AC_IO_ICCA_FLAG_FRONT_SENSOR;
    }

    if (sensor_state & (1 << BT_IO_EAM_SENSOR_STATE_BACK)) {
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

    if (encrypted) {
        size_t base_state_sz = sizeof(struct ac_io_icca_state);
        resp.cmd.nbytes = base_state_sz + 2;

        if (!icca->cipher_started) {
            log_warning("No crypto keys set for unit %d", icca->unit_no);
        } else {
            uint16_t crc = crc16_msb(resp.cmd.raw, base_state_sz, 0);

            resp.cmd.raw[base_state_sz + 0] = (crc >> 8) & 0xFF;
            resp.cmd.raw[base_state_sz + 1] = crc & 0xFF;

            ac_io_emu_icca_cipher(icca, resp.cmd.raw, 18);
        }
    }

    ac_io_emu_response_push(icca->emu, &resp, delay_us);
}

static void ac_io_emu_icca_cipher_shift_keys(struct ac_io_emu_icca *icca)
{
    uint32_t key4_old = icca->cipher_keys[3];
    uint32_t key_new = (key4_old << 11) ^ key4_old;
    uint32_t key1_old = icca->cipher_keys[0];

    // shift keys up
    icca->cipher_keys[3] = icca->cipher_keys[2];
    icca->cipher_keys[2] = icca->cipher_keys[1];
    icca->cipher_keys[1] = icca->cipher_keys[0];
    icca->cipher_keys[0] =
        ((((key1_old >> 11) ^ key_new) >> 8) ^ key_new ^ key1_old);
}

static void
ac_io_emu_icca_cipher(struct ac_io_emu_icca *icca, uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        uint8_t count4 = i % 4;
        // shift keys every 4 bytes
        if (count4 == 0) {
            ac_io_emu_icca_cipher_shift_keys(icca);
        }

        // process data
        data[i] =
            (uint8_t) (icca->cipher_keys[0] >> (((3 - count4) << 3)) ^ data[i]);
    }
}

static void ac_io_emu_icca_cipher_set_key(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req)
{
    struct ac_io_message resp;

    resp.addr = req->addr | AC_IO_RESPONSE_FLAG;
    resp.cmd.code = req->cmd.code;
    resp.cmd.seq_no = req->cmd.seq_no;
    resp.cmd.nbytes = 4; // 4 bytes, uint32_t

    uint32_t host_key = 0x00000000;
    host_key |= req->cmd.raw[0] << 24;
    host_key |= req->cmd.raw[1] << 16;
    host_key |= req->cmd.raw[2] << 8;
    host_key |= req->cmd.raw[3];

    // TODO: should probably RNG this
    uint32_t reader_key = 0x14243444;
    resp.cmd.raw[0] = (reader_key >> 24) & 0xFF;
    resp.cmd.raw[1] = (reader_key >> 16) & 0xFF;
    resp.cmd.raw[2] = (reader_key >> 8) & 0xFF;
    resp.cmd.raw[3] = (reader_key) & 0xFF;

    // so I looked these constants up, this isn't actually a secure key
    // generator it's actually Marsaglia's "KISS" algorithm with different
    // initial states
    icca->cipher_keys[0] = reader_key ^ 0x5491333;
    icca->cipher_keys[1] = host_key ^ 0x1F123BB5;
    icca->cipher_keys[2] = reader_key ^ 0x159A55E5;
    icca->cipher_keys[3] = host_key ^ 0x75BCD15;

    log_misc("keys set to: H: %08x R: %08x", host_key, reader_key);

    ac_io_emu_response_push(icca->emu, &resp, 0);
    icca->cipher_started = true;
}

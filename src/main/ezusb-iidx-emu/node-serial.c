#define LOG_MODULE "ezusb-iidx-emu-node-serial"

#include <windows.h>

#include "bemanitools/eamio.h"

#include "ezusb-iidx-emu/card-mag.c"
#include "ezusb-iidx-emu/node-serial.h"
#include "ezusb-iidx/serial-cmd.h"

#include "security/mcode.h"

#include "util/hex.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/thread.h"

#define CARD_ID_LEN 8

enum ezusb_iidx_emu_node_serial_serial_msg_cmd {
    CMD_H8_REQ = 0xAA,
    CMD_H8_RESP = 0xA5,
    CMD_NODE_REQ = 0x00,
    CMD_NODE_RESP = 0x01
};

enum ezusb_iidx_emu_node_serial_h8_cmd {
    H8_CMD_NODE_ENUM = 0x01,
    H8_CMD_GET_VERSION = 0x02,
    H8_CMD_PROG_EXEC = 0x03,
};

enum ezusb_iidx_emu_node_serial_node_cmd {
    NODE_CMD_CARD_INIT = 0x00,
    NODE_CMD_KEYBOARD_INIT = 0x10,
    NODE_CMD_CARD_RW_UNIT_GET_STATUS = 0x12,
    NODE_CMD_CARD_SLOT_SET_STATE = 0x14,
    NODE_CMD_CARD_WRITE = 0x16,
    NODE_CMD_CARD_READ = 0x18,
    /* Not sure what this does. Only used in combination
       with writing the card on the format call */
    NODE_CMD_CARD_FORMAT_COMPLETE = 0x1E,
    NODE_CMD_CARD_GET_STATUS = 0x20,
    NODE_CMD_KEYBOARD_GET_STATUS = 0x24,
    NODE_CMD_KEYBOARD_READ_DATA = 0x26,
    NODE_CMD_KEYBOARD_GET_BUFFER_SIZE = 0x27
};

enum ezusb_iidx_emu_node_serial_card_slot_state {
    /* Closes/locks the card slot if a card is inserted to avoid pulling out
       If no card is inserted, close means that no cards are accepted
       you have to call open first to accept a card */
    CARD_SLOT_STATE_CLOSE = 0,
    /* Open the card slot and allow a new card to be inserted and accepted */
    CARD_SLOT_STATE_OPEN = 1,
    /* Eject an inserted card */
    CARD_SLOT_STATE_EJECT = 2,
    /* Trigger card formating and write some data to it */
    CARD_SLOT_STATE_FORMAT = 3,
    /* Request reading data from the card. */
    CARD_SLOT_STATE_READ = 4,
    /* Request writing data to the card */
    CARD_SLOT_STATE_WRITE = 5,
};

#define NODE_SERIAL_MSG_HEADER_SIZE 4
#define MAG_CARD_DATA_SIZE 128

#pragma pack(push, 1)
struct ezusb_iidx_emu_node_serial_msg {
    /* Header part of message */
    uint8_t msg_cmd;
    uint8_t node_id;
    uint8_t node_cmd;
    uint8_t payload_len;

    union payload {
        struct ezusb_iidx_emu_node_serial_node_enum_req {
            uint8_t node_id_to_assign;
        } node_enum_req;

        struct ezusb_iidx_emu_node_serial_node_enum_resp {
            uint8_t total_nodes;
        } node_enum_resp;

        struct ezusb_iidx_emu_node_serial_get_version_resp {
            uint32_t type;
            uint8_t dup;
            uint8_t version_maj;
            uint8_t version_min;
            uint8_t version_rev;
            char comment[5];
        } get_version_resp;

        struct ezusb_iidx_emu_node_serial_keyboard_get_buffer_size_resp {
            uint16_t buffer_size_type;
        } keyboard_get_buffer_size_resp;

        struct ezusb_iidx_emu_node_serial_keyboard_read_data_req {
            uint8_t buffer_size_type;
        } keyboard_read_data_req;

        struct ezusb_iidx_emu_node_serial_keyboard_read_data_resp {
            /* dynamic size */
            uint8_t data[0xFF];
        } keyboard_read_data_resp;

        struct ezusb_iidx_emu_node_serial_card_slot_state_req {
            uint8_t ezusb_iidx_emu_node_serial_card_slot_state;
        } card_slot_state_req;

        struct ezusb_iidx_emu_node_serial_card_read_resp {
            /* 1 byte header + 128 byte card data */
            uint8_t header;
            uint8_t data[MAG_CARD_DATA_SIZE];
        } card_read_resp;

        struct ezusb_iidx_emu_node_serial_card_write_req {
            uint8_t data[MAG_CARD_DATA_SIZE];
        } card_write_req;

        /*
         * get_version_req
         * proc_exec_req
         * card_init_req
         * keyboard_init_req
         * card_get_status_req
         * keyboard_get_status_req
         * card_rw_unit_get_status_req
         * keyboard_get_buffer_size_req
         * card_read_req
         * card_format_complete_req
         */
        struct ezusb_iidx_emu_node_serial_common_req {
            /* no payload */
        } common_req;

        /*
         * proc_exec_resp
         * card_init_resp
         * keyboard_init_resp
         * card_get_status_resp
         * keyboard_get_status_resp
         * card_rw_unit_get_status_resp
         * card_slot_state_resp
         * card_write_resp
         * card_format_complete_resp
         */
        struct ezusb_iidx_emu_node_serial_common_status_resp {
            uint8_t status;
        } common_status_resp;
    };
};
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_node_enum_req) == 1,
    "ezusb_iidx_emu_node_serial_node_enum_req is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_node_enum_resp) == 1,
    "ezusb_iidx_emu_node_serial_node_enum_resp is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_get_version_resp) == 13,
    "ezusb_iidx_emu_node_serial_get_version_resp is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_keyboard_get_buffer_size_resp) == 2,
    "ezusb_iidx_emu_node_serial_keyboard_get_buffer_size_resp is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_keyboard_read_data_req) == 1,
    "ezusb_iidx_emu_node_serial_keyboard_read_data_req is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_keyboard_read_data_resp) == 255,
    "ezusb_iidx_emu_node_serial_keyboard_read_data_resp is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_card_slot_state_req) == 1,
    "ezusb_iidx_emu_node_serial_card_slot_state_req is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_card_read_resp) == 1 + MAG_CARD_DATA_SIZE,
    "ezusb_iidx_emu_node_serial_card_read_resp is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_card_write_req) == MAG_CARD_DATA_SIZE,
    "ezusb_iidx_emu_node_serial_card_write_req is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_common_req) == 0,
    "ezusb_iidx_emu_node_serial_common_req is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_common_status_resp) == 1,
    "ezusb_iidx_emu_node_serial_common_status_resp is the wrong size");
_Static_assert(
    sizeof(struct ezusb_iidx_emu_node_serial_msg) == 259,
    "ezusb_iidx_emu_node_serial_msg is the wrong size");
#pragma pack(pop)

/* ------------------------------------------------------------------------- */

enum ezusb_iidx_emu_node_serial_emu_card_slot_state {
    EMU_CARD_SLOT_STATE_CLOSE = 0,
    EMU_CARD_SLOT_STATE_OPEN = 1,
    EMU_CARD_SLOT_STATE_EJECT = 2,
    EMU_CARD_SLOT_STATE_READ = 3,
};

enum ezusb_iidx_emu_node_serial_emu_state {
    EMU_STATE_UNINIT = 0,
    EMU_STATE_INIT = 1,
    EMU_STATE_LOOP = 2,
    EMU_STATE_REQ_CARD = 3,
    EMU_STATE_CARD_READ = 4,
    EMU_STATE_ERROR = 5,
};

struct ezusb_iidx_emu_node_serial_emulation_state {
    volatile long state_current;
    volatile LONG keypad_buf;

    bool card_slot_sensor_front;
    bool card_slot_sensor_back;

    uint8_t ezusb_iidx_emu_node_serial_card_slot_state;

    CRITICAL_SECTION card_cs;
    uint8_t card_id[CARD_ID_LEN];
};

/* ------------------------------------------------------------------------- */

static const char
    ezusb_iidx_emu_node_serial_eamio_mapping[EAM_IO_KEYPAD_COUNT] = {
        '0', '1', '4', '7', 'O', '2', '5', '8', 'E', '3', '6', '9'};

static const uint8_t HEADER_BYTE = 0xAA;

static bool ezusb_iidx_emu_node_serial_read_buf_busy;
static bool ezusb_iidx_emu_node_serial_write_buf_busy;
static uint8_t ezusb_iidx_emu_node_serial_read_buf[512];
static uint8_t ezusb_iidx_emu_node_serial_read_buf_page;
static uint16_t ezusb_iidx_emu_node_serial_read_buf_data_len;
static uint8_t ezusb_iidx_emu_node_serial_write_buf[512];
static uint8_t ezusb_iidx_emu_node_serial_wrote_buf_page;
static uint16_t ezusb_iidx_emu_node_serial_write_buf_data_len;

static uint8_t ezusb_iidx_emu_node_serial_card_attr_type;
static bool ezusb_iidx_emu_node_serial_card_attr_used = true;
static char ezusb_iidx_emu_node_serial_card_attr_version[3] =
    SECURITY_MCODE_GAME_IIDX_9;

static uint8_t *ezusb_iidx_emu_node_serial_write_loopback_card_buf[2];

static int ezusb_iidx_emu_node_serial_emu_thread = -1;
static struct ezusb_iidx_emu_node_serial_emulation_state
    ezusb_iidx_emu_node_serial_emulation_state[2];

static void ezusb_iidx_emu_node_serial_dump_buf_log(
    const char *header_msg, const uint8_t *buffer, uint16_t length);

static void ezusb_iidx_emu_node_serial_exec_req_resp(
    const uint8_t *buffer_in,
    uint16_t buffer_in_length,
    uint8_t *buffer_out,
    uint16_t *buffer_out_length);

static int ezusb_iidx_emu_node_serial_emu_thread_proc(void *ctx);

/* ------------------------------------------------------------------------- */

void ezusb_iidx_emu_node_serial_init(void)
{
    ezusb_iidx_emu_node_serial_read_buf_busy = false;
    ezusb_iidx_emu_node_serial_write_buf_busy = false;
    memset(
        ezusb_iidx_emu_node_serial_read_buf,
        0xFF,
        sizeof(ezusb_iidx_emu_node_serial_read_buf));
    ezusb_iidx_emu_node_serial_read_buf_page = 0;
    ezusb_iidx_emu_node_serial_read_buf_data_len = 0;
    memset(
        ezusb_iidx_emu_node_serial_write_buf,
        0xFF,
        sizeof(ezusb_iidx_emu_node_serial_write_buf));
    ezusb_iidx_emu_node_serial_wrote_buf_page = 0;
    ezusb_iidx_emu_node_serial_write_buf_data_len = 0;

    memset(
        ezusb_iidx_emu_node_serial_emulation_state,
        0,
        sizeof(struct ezusb_iidx_emu_node_serial_emulation_state) * 2);

    for (uint8_t i = 0; i < 2; i++) {
        InitializeCriticalSection(
            &ezusb_iidx_emu_node_serial_emulation_state[i].card_cs);
    }

    ezusb_iidx_emu_node_serial_emu_thread = thread_create(
        ezusb_iidx_emu_node_serial_emu_thread_proc, NULL, 0x4000, 0);
}

uint8_t ezusb_iidx_emu_node_serial_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_SERIAL_CMD_READ_BUFFER:
            // log_misc("EZUSB_SERIAL_CMD_READ_BUFFER");
            ezusb_iidx_emu_node_serial_read_buf_busy = false;
            /* always reset page count on req */
            ezusb_iidx_emu_node_serial_read_buf_page = 0;
            return EZUSB_IIDX_SERIAL_CMD_STATUS_OK;

        case EZUSB_IIDX_SERIAL_CMD_WRITE_BUFFER:
            // log_misc("EZUSB_SERIAL_CMD_WRITE_BUFFER");
            ezusb_iidx_emu_node_serial_write_buf_busy = false;
            /* always reset page count on req */
            ezusb_iidx_emu_node_serial_wrote_buf_page = 0;
            return EZUSB_IIDX_SERIAL_CMD_STATUS_OK;

        case EZUSB_IIDX_SERIAL_CMD_CLEAR_READ_BUFFER:
            // log_misc("EZUSB_SERIAL_CMD_CLEAR_READ_BUFFER");
            ezusb_iidx_emu_node_serial_read_buf_busy = false;
            /* always reset page count on req */
            ezusb_iidx_emu_node_serial_read_buf_page = 0;
            return EZUSB_IIDX_SERIAL_CMD_STATUS_OK;

        case EZUSB_IIDX_SERIAL_CMD_CLEAR_WRITE_BUFFER:
            // log_misc("EZUSB_SERIAL_CMD_CLEAR_WRITE_BUFFER");
            ezusb_iidx_emu_node_serial_write_buf_busy = false;
            /* always reset page count on req */
            ezusb_iidx_emu_node_serial_wrote_buf_page = 0;
            return EZUSB_IIDX_SERIAL_CMD_STATUS_OK;

        default:
            log_warning("Unrecognised serial command: %02x", cmd_id);
            return EZUSB_IIDX_SERIAL_CMD_STATUS_FAULT;
    }
}

bool ezusb_iidx_emu_node_serial_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    uint16_t buffer_offset = 0;
    uint16_t data_length = 0;

    /* id to get accepted */
    pkg->node = 0x42;
    pkg->page = ezusb_iidx_emu_node_serial_read_buf_page;

    memset(pkg->payload, 0xFF, sizeof(pkg->payload));

    /* calc data size */
    buffer_offset =
        ezusb_iidx_emu_node_serial_read_buf_page * sizeof(pkg->payload);

    if (ezusb_iidx_emu_node_serial_read_buf_data_len >= sizeof(pkg->payload)) {
        data_length = sizeof(pkg->payload);
        ezusb_iidx_emu_node_serial_read_buf_data_len -= sizeof(pkg->payload);
        ezusb_iidx_emu_node_serial_read_buf_page++;
    } else {
        data_length = ezusb_iidx_emu_node_serial_read_buf_data_len;
        pkg->page = 0x40 + data_length;
        ezusb_iidx_emu_node_serial_read_buf_data_len = 0;
    }

    memcpy(
        pkg->payload,
        ezusb_iidx_emu_node_serial_read_buf + buffer_offset,
        data_length);

    return true;
}

bool ezusb_iidx_emu_node_serial_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    bool execute = false;
    uint16_t buffer_offset = 0;
    uint16_t data_length = 0;

    buffer_offset =
        ezusb_iidx_emu_node_serial_wrote_buf_page * sizeof(pkg->payload);

    /* the stuff getting received here is a serial stream
       which is terminated by a size >= 0x42
       if the page var is < 0x42, it indicates the page */

    if (pkg->page >= 0x42) {
        data_length = pkg->page - 0x42;
        execute = true;
    } else {
        /* full page */
        data_length = sizeof(pkg->payload);
        ezusb_iidx_emu_node_serial_wrote_buf_page++;
    }

    ezusb_iidx_emu_node_serial_write_buf_data_len += data_length;
    memcpy(
        ezusb_iidx_emu_node_serial_write_buf + buffer_offset,
        pkg->payload,
        data_length);

    /* serial buffer is filled, execute request/response chain
       dunno if this is a good spot to do this... */

    if (execute) {
        ezusb_iidx_emu_node_serial_exec_req_resp(
            ezusb_iidx_emu_node_serial_write_buf,
            ezusb_iidx_emu_node_serial_write_buf_data_len,
            ezusb_iidx_emu_node_serial_read_buf,
            &ezusb_iidx_emu_node_serial_read_buf_data_len);

        /* not sure, but there seems to be a bug with the game
           not resetting the buffer in the init phase
           let us make sure we got this when receiving the serial stream
           terminator and having executed something */
        ezusb_iidx_emu_node_serial_write_buf_data_len = 0;
    }

    return true;
}

/* ------------------------------------------------------------------------- */

bool ezusb_iidx_emu_node_serial_read_buffer_busy(void)
{
    return ezusb_iidx_emu_node_serial_read_buf_busy;
}

bool ezusb_iidx_emu_node_serial_write_buffer_busy(void)
{
    return ezusb_iidx_emu_node_serial_write_buf_busy;
}

void ezusb_iidx_emu_node_serial_set_card_attributes(
    uint8_t card_type, bool used_card, const char *card_version)
{
    ezusb_iidx_emu_node_serial_card_attr_type = card_type > 4 ? 4 : card_type;
    ezusb_iidx_emu_node_serial_card_attr_used = used_card;
    memcpy(
        ezusb_iidx_emu_node_serial_card_attr_version,
        card_version,
        SECURITY_MCODE_GAME_LEN);
}

/* ------------------------------------------------------------------------- */

static void ezusb_iidx_emu_node_serial_dump_buf_log(
    const char *header_msg, const uint8_t *buffer, uint16_t length)
{
    char tmp[4096];
    memset(tmp, 0, sizeof(tmp));
    hex_encode_uc(buffer, length, tmp, sizeof(tmp));
    log_misc(">>> Serial data dump, %s (%d): %s", header_msg, length, tmp);
}

static uint8_t
calc_serial_buffer_checksum(const uint8_t *buffer, uint16_t length)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++)
        checksum += buffer[i];

    return checksum;
}

static struct ezusb_iidx_emu_node_serial_msg *create_generic_node_response_ok(
    const struct ezusb_iidx_emu_node_serial_msg *msg_in, uint16_t *msg_out_len)
{
    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + sizeof(struct ezusb_iidx_emu_node_serial_common_status_resp);
    struct ezusb_iidx_emu_node_serial_msg *resp = xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
    resp->msg_cmd = CMD_NODE_RESP;
    resp->node_id = msg_in->node_id;
    resp->node_cmd = msg_in->node_cmd;
    resp->payload_len = 1;
    resp->common_status_resp.status = 0;
    return resp;
}

static struct ezusb_iidx_emu_node_serial_msg *process_serial_msg(
    const struct ezusb_iidx_emu_node_serial_msg *msg_in,
    uint16_t msg_in_len,
    uint16_t *msg_out_len)
{
    // log_misc("!!! Serial processing msg_cmd %02X, node_cmd %02X, node_id
    // %02X",
    //            msg_in->msg_cmd, msg_in->node_cmd, msg_in->node_id);

    switch (msg_in->msg_cmd) {
        case CMD_H8_REQ: {
            switch (msg_in->node_cmd) {
                case H8_CMD_NODE_ENUM: {
                    log_misc("H8_CMD_NODE_ENUM");
                    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + sizeof(struct ezusb_iidx_emu_node_serial_node_enum_resp);
                    struct ezusb_iidx_emu_node_serial_msg *resp =
                        xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
                    resp->msg_cmd = CMD_H8_RESP;
                    /* This response is not from a specific node, id needs to be
                     * 0 */
                    resp->node_id = 0;
                    resp->node_cmd = msg_in->node_cmd;
                    resp->payload_len = 1;
                    resp->node_enum_resp.total_nodes = 2;
                    return resp;
                }

                case H8_CMD_GET_VERSION: {
                    log_misc("H8_CMD_GET_VERSION: %d", msg_in->node_id);
                    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + sizeof(struct ezusb_iidx_emu_node_serial_get_version_resp);
                    struct ezusb_iidx_emu_node_serial_msg *resp =
                        xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
                    resp->msg_cmd = CMD_H8_RESP;
                    resp->node_id = msg_in->node_id;
                    resp->node_cmd = msg_in->node_cmd;
                    /* Payload length is not 13 here, needs to 5 (comment
                       length?) Might be a bug in the ezusb code */
                    resp->payload_len = 5;
                    resp->get_version_resp.type = 0x03;
                    resp->get_version_resp.dup = 0x00;
                    resp->get_version_resp.version_maj = 1;
                    resp->get_version_resp.version_min = 6;
                    resp->get_version_resp.version_rev = 0;
                    resp->get_version_resp.comment[0] = 'I';
                    resp->get_version_resp.comment[1] = 'C';
                    resp->get_version_resp.comment[2] = 'C';
                    resp->get_version_resp.comment[3] = 'A';
                    resp->get_version_resp.comment[4] = '\0';
                    return resp;
                }

                case H8_CMD_PROG_EXEC: {
                    log_misc("H8_CMD_PROG_EXEC: %d", msg_in->node_id);

                    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + sizeof(struct ezusb_iidx_emu_node_serial_common_status_resp);
                    struct ezusb_iidx_emu_node_serial_msg *resp =
                        xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
                    resp->msg_cmd = CMD_H8_RESP;
                    resp->node_id = msg_in->node_id;
                    resp->node_cmd = msg_in->node_cmd;
                    resp->payload_len = 1;

                    /* init reader emulation here if not initialized, yet */
                    uint8_t state = ezusb_iidx_emu_node_serial_emulation_state
                                        [msg_in->node_id - 1]
                                            .state_current;

                    if (state == EMU_STATE_UNINIT) {
                        InterlockedExchange(
                            &ezusb_iidx_emu_node_serial_emulation_state
                                 [msg_in->node_id - 1]
                                     .state_current,
                            EMU_STATE_INIT);

                        /* this should not take too long, wait */
                        while (true) {
                            state = ezusb_iidx_emu_node_serial_emulation_state
                                        [msg_in->node_id - 1]
                                            .state_current;

                            if (state == EMU_STATE_LOOP) {
                                resp->common_status_resp.status = 0;
                                break;
                            } else if (state == EMU_STATE_ERROR) {
                                resp->common_status_resp.status = 0xFF;
                                break;
                            }

                            Sleep(10);
                        }
                    } else {
                        if (state == EMU_STATE_ERROR) {
                            resp->common_status_resp.status = 0xFF;
                        } else {
                            resp->common_status_resp.status = 0;
                        }
                    }

                    return resp;
                }

                default:
                    log_warning(
                        "Serial invalid node cmd on h8 req: %02X",
                        msg_in->node_cmd);
                    ezusb_iidx_emu_node_serial_dump_buf_log(
                        "", (const uint8_t *) msg_in, msg_in_len);
                    return NULL;
            }
        }

        case CMD_NODE_REQ: {
            switch (msg_in->node_cmd) {
                case NODE_CMD_CARD_INIT:
                    log_misc("NODE_CMD_CARD_INIT: %d", msg_in->node_id);
                    return create_generic_node_response_ok(msg_in, msg_out_len);

                case NODE_CMD_KEYBOARD_INIT:
                    log_misc("NODE_CMD_KEYBOARD_INIT: %d", msg_in->node_id);
                    return create_generic_node_response_ok(msg_in, msg_out_len);

                case NODE_CMD_KEYBOARD_GET_STATUS:
                    return create_generic_node_response_ok(msg_in, msg_out_len);

                case NODE_CMD_CARD_GET_STATUS:
                    return create_generic_node_response_ok(msg_in, msg_out_len);

                case NODE_CMD_CARD_RW_UNIT_GET_STATUS: {
                    /* report front and back sensor states */

                    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + sizeof(struct ezusb_iidx_emu_node_serial_common_status_resp);
                    struct ezusb_iidx_emu_node_serial_msg *resp =
                        xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
                    resp->msg_cmd = CMD_NODE_RESP;
                    resp->node_id = msg_in->node_id;
                    resp->node_cmd = msg_in->node_cmd;
                    resp->payload_len = 1;
                    resp->common_status_resp.status = 0;

                    if (ezusb_iidx_emu_node_serial_emulation_state
                            [msg_in->node_id - 1]
                                .state_current == EMU_STATE_ERROR) {
                        resp->common_status_resp.status = 0xFF;
                    } else {
                        if (ezusb_iidx_emu_node_serial_emulation_state
                                [msg_in->node_id - 1]
                                    .card_slot_sensor_back) {
                            /* back, triggers card inserted */
                            resp->common_status_resp.status |= 128;
                            /* needs to be set if either front or back is
                             * triggered */
                            resp->common_status_resp.status |= 2;
                        }

                        if (ezusb_iidx_emu_node_serial_emulation_state
                                [msg_in->node_id - 1]
                                    .card_slot_sensor_front) {
                            /* front */
                            resp->common_status_resp.status |= 64;
                            /* needs to be set if either front or back is
                             * triggered */
                            resp->common_status_resp.status |= 2;
                        }
                    }

                    return resp;
                }

                case NODE_CMD_CARD_SLOT_SET_STATE: {
                    switch (msg_in->card_slot_state_req
                                .ezusb_iidx_emu_node_serial_card_slot_state) {
                        case CARD_SLOT_STATE_CLOSE:
                            log_misc(
                                "CARD_SLOT_STATE_CLOSE, node %d",
                                msg_in->node_id);

                            ezusb_iidx_emu_node_serial_emulation_state
                                [msg_in->node_id - 1]
                                    .ezusb_iidx_emu_node_serial_card_slot_state =
                                EMU_CARD_SLOT_STATE_CLOSE;

                            break;

                        case CARD_SLOT_STATE_OPEN:
                            log_misc(
                                "CARD_SLOT_STATE_OPEN, node %d",
                                msg_in->node_id);

                            ezusb_iidx_emu_node_serial_emulation_state
                                [msg_in->node_id - 1]
                                    .ezusb_iidx_emu_node_serial_card_slot_state =
                                EMU_CARD_SLOT_STATE_OPEN;

                            break;

                        case CARD_SLOT_STATE_EJECT:
                            log_misc(
                                "CARD_SLOT_STATE_EJECT, node %d",
                                msg_in->node_id);

                            ezusb_iidx_emu_node_serial_emulation_state
                                [msg_in->node_id - 1]
                                    .ezusb_iidx_emu_node_serial_card_slot_state =
                                EMU_CARD_SLOT_STATE_EJECT;

                            break;

                        case CARD_SLOT_STATE_FORMAT:
                            log_misc(
                                "CARD_SLOT_STATE_FORMAT, node %d",
                                msg_in->node_id);
                            break;

                        case CARD_SLOT_STATE_READ:
                            log_misc(
                                "CARD_SLOT_STATE_READ, node %d",
                                msg_in->node_id);

                            InterlockedExchange(
                                &ezusb_iidx_emu_node_serial_emulation_state
                                     [msg_in->node_id - 1]
                                         .state_current,
                                EMU_STATE_REQ_CARD);

                            ezusb_iidx_emu_node_serial_emulation_state
                                [msg_in->node_id - 1]
                                    .ezusb_iidx_emu_node_serial_card_slot_state =
                                EMU_CARD_SLOT_STATE_READ;

                            break;

                        case CARD_SLOT_STATE_WRITE:
                            log_misc(
                                "CARD_SLOT_STATE_WRITE, node %d",
                                msg_in->node_id);
                            break;

                        default:
                            log_warning(
                                "Invalid card slot state %d for node %d",
                                msg_in->card_slot_state_req
                                    .ezusb_iidx_emu_node_serial_card_slot_state,
                                msg_in->node_id);
                    }

                    return create_generic_node_response_ok(msg_in, msg_out_len);
                }

                case NODE_CMD_CARD_READ: {
                    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + sizeof(struct ezusb_iidx_emu_node_serial_card_read_resp);
                    struct ezusb_iidx_emu_node_serial_msg *resp =
                        xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
                    resp->msg_cmd = CMD_NODE_RESP;
                    resp->node_id = msg_in->node_id;
                    resp->node_cmd = msg_in->node_cmd;
                    /* game expects 24 here, even more data is sent */
                    resp->payload_len = 24;
                    /* fixed header byte */
                    resp->card_read_resp.header = 0x48;
                    memset(resp->card_read_resp.data, 0x00, MAG_CARD_DATA_SIZE);

                    /* loop back read for write (see write) */

                    if (ezusb_iidx_emu_node_serial_write_loopback_card_buf
                            [msg_in->node_id - 1] == NULL) {
                        struct ezusb_iidx_emu_card_mag_data *card =
                            (struct ezusb_iidx_emu_card_mag_data *)
                                resp->card_read_resp.data;

                        uint8_t state =
                            ezusb_iidx_emu_node_serial_emulation_state
                                [msg_in->node_id - 1]
                                    .state_current;

                        /* wait for card to get read */
                        while (state == EMU_STATE_REQ_CARD) {
                            state = ezusb_iidx_emu_node_serial_emulation_state
                                        [msg_in->node_id - 1]
                                            .state_current;
                            Sleep(5);
                        }

                        if (state == EMU_STATE_CARD_READ) {
                            uint8_t card_invert[CARD_ID_LEN];

                            EnterCriticalSection(
                                &ezusb_iidx_emu_node_serial_emulation_state
                                     [msg_in->node_id - 1]
                                         .card_cs);

                            log_info(
                                "Read card %02X%02X%02X%02X%02X%02X%02X%02X",
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[0],
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[1],
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[2],
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[3],
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[4],
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[5],
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[6],
                                ezusb_iidx_emu_node_serial_emulation_state
                                    [msg_in->node_id - 1]
                                        .card_id[7]);

                            /* game flips endianess */
                            for (uint8_t i = 0; i < CARD_ID_LEN; i++) {
                                card_invert[7 - i] =
                                    ezusb_iidx_emu_node_serial_emulation_state
                                        [msg_in->node_id - 1]
                                            .card_id[i];
                            }

                            ezusb_iidx_emu_card_mag_generate_data(
                                card,
                                card_invert,
                                ezusb_iidx_emu_node_serial_card_attr_type,
                                ezusb_iidx_emu_node_serial_card_attr_used,
                                ezusb_iidx_emu_node_serial_card_attr_version);

                            /* ezusb_iidx_emu_node_serial_dump_buf_log("card
                               data", resp->card_read_resp.data, sizeof(struct
                               ezusb_iidx_emu_card_mag_data)); */

                            InterlockedExchange(
                                &ezusb_iidx_emu_node_serial_emulation_state
                                     [msg_in->node_id - 1]
                                         .state_current,
                                EMU_STATE_LOOP);

                            LeaveCriticalSection(
                                &ezusb_iidx_emu_node_serial_emulation_state
                                     [msg_in->node_id - 1]
                                         .card_cs);

                        } else if (state == EMU_STATE_ERROR) {
                            /* this indicates an error */
                            resp->payload_len = 1;
                            resp->card_read_resp.data[0] = 0xFF;
                        }
                    } else {
                        log_misc("Reading loopback card buffer");
                        memcpy(
                            resp->card_read_resp.data,
                            ezusb_iidx_emu_node_serial_write_loopback_card_buf
                                [msg_in->node_id - 1],
                            MAG_CARD_DATA_SIZE);

                        free(ezusb_iidx_emu_node_serial_write_loopback_card_buf
                                 [msg_in->node_id - 1]);
                        ezusb_iidx_emu_node_serial_write_loopback_card_buf
                            [msg_in->node_id - 1] = NULL;
                    }

                    return resp;
                }

                case NODE_CMD_CARD_WRITE: {
                    /* i hate this solution, but it gets rid of the problem that
                       we don't know the whole magnetic card spec and the game
                       is ok with just a bunch of them when read only
                       If a write occurs (for example if the card is registered
                       for a new player) we will cashe the data which arrives on
                       the write and loop it back on the next read (next read
                       only) */

                    if (ezusb_iidx_emu_node_serial_write_loopback_card_buf
                            [msg_in->node_id - 1] != NULL) {
                        free(ezusb_iidx_emu_node_serial_write_loopback_card_buf
                                 [msg_in->node_id - 1]);
                    }

                    ezusb_iidx_emu_node_serial_write_loopback_card_buf
                        [msg_in->node_id - 1] = xmalloc(MAG_CARD_DATA_SIZE);
                    memcpy(
                        ezusb_iidx_emu_node_serial_write_loopback_card_buf
                            [msg_in->node_id - 1],
                        msg_in->card_write_req.data,
                        MAG_CARD_DATA_SIZE);

                    return create_generic_node_response_ok(msg_in, msg_out_len);
                }

                case NODE_CMD_CARD_FORMAT_COMPLETE: {
                    /* just reply, no handling of data necessary */
                    return create_generic_node_response_ok(msg_in, msg_out_len);
                }

                case NODE_CMD_KEYBOARD_GET_BUFFER_SIZE: {
                    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + sizeof(struct ezusb_iidx_emu_node_serial_keyboard_get_buffer_size_resp);
                    struct ezusb_iidx_emu_node_serial_msg *resp =
                        xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
                    resp->msg_cmd = CMD_NODE_RESP;
                    resp->node_id = msg_in->node_id;
                    resp->node_cmd = msg_in->node_cmd;
                    resp->payload_len = 2;
                    resp->common_status_resp.status = 0;

                    /* read buffer sizes. the size is getting converted
                       and the converted value sent to us again on the read call
                       0 -> 0 no data to send
                       1 -> 1
                       2 -> 2
                       3 -> 3-4 i.e. max buffer size 4
                       4 -> 5-8 i.e. max buffer size 8
                       5 -> 9-16 i.e. max buffer size 16
                       6 -> 17-32 i.e. max buffer size 32
                       7 -> 33-64 i.e. max buffer size 64 */

                    /* The card reader driver is really bad performance wise
                       so it's even slower if we use a ring buffer like
                       data structure. It's easier to use a single byte
                       buffer and even faster to handle for the code here */

                    uint16_t bufferSize = 0;
                    uint8_t node = msg_in->node_id - 1;
                    if (ezusb_iidx_emu_node_serial_emulation_state[node]
                            .keypad_buf != 0) {
                        bufferSize = 1;
                    }

                    bufferSize = bufferSize >= 64 ? 64 : bufferSize;

                    /* convert */
                    if (bufferSize < 3) {
                        resp->keyboard_get_buffer_size_resp.buffer_size_type =
                            bufferSize;
                    } else if (bufferSize <= 4) {
                        resp->keyboard_get_buffer_size_resp.buffer_size_type =
                            3;
                    } else if (bufferSize <= 8) {
                        resp->keyboard_get_buffer_size_resp.buffer_size_type =
                            4;
                    } else if (bufferSize <= 16) {
                        resp->keyboard_get_buffer_size_resp.buffer_size_type =
                            5;
                    } else if (bufferSize <= 32) {
                        resp->keyboard_get_buffer_size_resp.buffer_size_type =
                            6;
                    } else {
                        /* avoid overflow, clip everything else */
                        resp->keyboard_get_buffer_size_resp.buffer_size_type =
                            7;
                    }

                    return resp;
                }

                case NODE_CMD_KEYBOARD_READ_DATA: {
                    *msg_out_len = NODE_SERIAL_MSG_HEADER_SIZE + 2;
                    struct ezusb_iidx_emu_node_serial_msg *resp =
                        xmalloc(sizeof(struct ezusb_iidx_emu_node_serial_msg));
                    resp->msg_cmd = CMD_NODE_RESP;
                    resp->node_id = msg_in->node_id;
                    resp->node_cmd = msg_in->node_cmd;
                    /* has to be the same as the size returned */
                    resp->payload_len =
                        msg_in->keyboard_read_data_req.buffer_size_type;

                    uint8_t maxBufferSize = 0;

                    /* convert back */
                    switch (msg_in->keyboard_read_data_req.buffer_size_type) {
                        case 0:
                            maxBufferSize = 0;
                            break;
                        case 1:
                            maxBufferSize = 1;
                            break;
                        case 2:
                            maxBufferSize = 2;
                            break;
                        case 3:
                            maxBufferSize = 4;
                            break;
                        case 4:
                            maxBufferSize = 8;
                            break;
                        case 5:
                            maxBufferSize = 16;
                            break;
                        case 6:
                            maxBufferSize = 32;
                            break;
                        case 7:
                            maxBufferSize = 64;
                            break;
                        default:
                            log_fatal(
                                "Invalid buffer size %d for reading keyboard "
                                "data.",
                                msg_in->keyboard_read_data_req
                                    .buffer_size_type);
                            break;
                    }

                    memset(
                        resp->keyboard_read_data_resp.data, 0, maxBufferSize);

                    if (maxBufferSize > 0) {
                        uint8_t node = msg_in->node_id - 1;
                        resp->keyboard_read_data_resp
                            .data[0] = InterlockedExchange(
                            &ezusb_iidx_emu_node_serial_emulation_state[node]
                                 .keypad_buf,
                            0);
                    }

                    return resp;
                }

                default:
                    log_warning(
                        "Serial invalid node cmd on node req: %02X",
                        msg_in->node_cmd);
                    ezusb_iidx_emu_node_serial_dump_buf_log(
                        "", (const uint8_t *) msg_in, msg_in_len);
                    return NULL;
            }
        }

        default:
            log_warning("Serial invalid msg cmd id %02X", msg_in->msg_cmd);
            ezusb_iidx_emu_node_serial_dump_buf_log(
                "", (const uint8_t *) msg_in, msg_in_len);
            return NULL;
    }
}

static void ezusb_iidx_emu_node_serial_exec_req_resp(
    const uint8_t *buffer_in,
    uint16_t buffer_in_length,
    uint8_t *buffer_out,
    uint16_t *buffer_out_length)
{
    /* data layout
       index 0: 0xAA header byte always
       index 1 to (length-1) - 1 excluding: payload
       index (length-1) - 1: checksum byte -> add up all data bytes between
       first and last byte

       no escape byte like on newer readers here */

    /* Before we do any processing:
       Filter garbage/broken messages. Maybe caused by buggy code (?) some
       messages during the uart init phase start with 0x00 and don't have a
       valid checksum. However, the game wants a response for that but doesn't
       care about the contents so we just throw the trash back where it came
       from */

    /*
    ezusb_iidx_emu_node_serial_dump_buf_log("request packed",
           (const uint8_t*) buffer_in, buffer_in_length);
    */

    if ((buffer_in_length > 0 && buffer_in[0] == 0x00) ||
        /* That's some uart init/reset message. Response expected, same message
         */
        (buffer_in_length == 4 && buffer_in[0] == 0xAA)) {
        *buffer_out_length = buffer_in_length;
        memcpy(buffer_out, buffer_in, buffer_in_length);
        return;
    }

    /* check header */

    if (buffer_in_length > 0 && buffer_in[0] != HEADER_BYTE) {
        log_warning("Invalid serial message, not starting with header byte");
        ezusb_iidx_emu_node_serial_dump_buf_log(
            "", (const uint8_t *) buffer_in, buffer_in_length);
        *buffer_out_length = 0;
        return;
    }

    /* Checksum of each message is all data between header and checksum
       byte (last byte of msg) */

    if (buffer_in[buffer_in_length - 1] !=
        calc_serial_buffer_checksum(buffer_in + 1, buffer_in_length - 2)) {
        log_warning("Serial message invalid checksum");
        ezusb_iidx_emu_node_serial_dump_buf_log(
            "", buffer_in, buffer_in_length);
        *buffer_out_length = 0;
        return;
    }

    const uint8_t *msg_buf_in = &buffer_in[1];
    uint16_t msg_buf_in_len = buffer_in_length - 2;

    /* Exception: catch uart init/reset message, game expects to respond with
       the same message this message is sent/written multiple times and read
       back once only by the game */

    if (msg_buf_in_len == 2 && msg_buf_in[0] == HEADER_BYTE &&
        msg_buf_in[1] == HEADER_BYTE) {
        uint8_t *msg_buf_out = &buffer_out[1];
        uint16_t msg_buf_out_len = 0;

        msg_buf_out[0] = HEADER_BYTE;
        msg_buf_out[1] = HEADER_BYTE;
        msg_buf_out_len = 2;

        buffer_out[0] = HEADER_BYTE;
        buffer_out[msg_buf_out_len + 1] =
            calc_serial_buffer_checksum(msg_buf_out, msg_buf_out_len);
        *buffer_out_length = msg_buf_out_len + 2;
    } else {
        /* Wtf, really? */
        /* All messages except the uart init/reset and node enum message
           must send the request they respond to along... */
        const struct ezusb_iidx_emu_node_serial_msg *msg_in =
            (const struct ezusb_iidx_emu_node_serial_msg *) msg_buf_in;

        if (msg_in->msg_cmd == CMD_H8_REQ &&
            msg_in->node_cmd == H8_CMD_NODE_ENUM) {
            uint8_t *msg_buf_out = &buffer_out[1];
            uint16_t msg_buf_out_len = 0;
            struct ezusb_iidx_emu_node_serial_msg *msg_out = process_serial_msg(
                (const struct ezusb_iidx_emu_node_serial_msg *) msg_buf_in,
                msg_buf_in_len,
                &msg_buf_out_len);

            if (msg_out == NULL) {
                *buffer_out_length = 0;
                return;
            }

            memcpy(msg_buf_out, msg_out, msg_buf_out_len);
            free(msg_out);

            /* pack serial out data, add header and checksum */
            buffer_out[0] = HEADER_BYTE;
            buffer_out[msg_buf_out_len + 1] =
                calc_serial_buffer_checksum(msg_buf_out, msg_buf_out_len);
            *buffer_out_length = msg_buf_out_len + 2;
        } else {
            /* everything else that's not fucked up */
            uint8_t *msg_buf_out = &buffer_out[buffer_in_length + 1];
            uint16_t msg_buf_out_len = 0;
            struct ezusb_iidx_emu_node_serial_msg *msg_out = process_serial_msg(
                (const struct ezusb_iidx_emu_node_serial_msg *) msg_buf_in,
                msg_buf_in_len,
                &msg_buf_out_len);

            if (msg_out == NULL) {
                *buffer_out_length = 0;
                return;
            }

            /* Copy request followed by packed response */
            memcpy(buffer_out, buffer_in, buffer_in_length);

            memcpy(msg_buf_out, msg_out, msg_buf_out_len);
            free(msg_out);

            /* pack serial out data (after the request which needs to be part
                 of the full response buffer), add header and checksum */
            buffer_out[buffer_in_length] = HEADER_BYTE;
            buffer_out[buffer_in_length + msg_buf_out_len + 1] =
                calc_serial_buffer_checksum(msg_buf_out, msg_buf_out_len);

            *buffer_out_length = buffer_in_length + msg_buf_out_len + 2;
        }
    }

    /*
    ezusb_iidx_emu_node_serial_dump_buf_log("response packed",
           (const uint8_t*) buffer_out, *buffer_out_length);
    */
}

/* ------------------------------------------------------------------------- */

static uint8_t convert_keyboard_char(char c)
{
    switch (c) {
        case '0':
            return 0x70;
        case '1':
            return 0x69;
        case '2':
            return 0x72;
        case '3':
            return 0x7A;
        case '4':
            return 0x6B;
        case '5':
            return 0x73;
        case '6':
            return 0x74;
        case '7':
            return 0x6C;
        case '8':
            return 0x75;
        case '9':
            return 0x7D;
        /* seems like 00 is not supported, use 0 instead  */
        case 'O':
            return 0x70;
        /* that's deleting/backspace */
        case 'E':
            return 0x66;
        default:
            return 0;
    }
    /* more codes, don't seem to be used
       resp->m_data[10] = 0x77;
       resp->m_data[11] = 0x4A;
       resp->m_data[12] = 0x7C;
       resp->m_data[13] = 0x66;
       resp->m_data[14] = 0x7B;
       resp->m_data[15] = 0x76;
      resp->m_data[16] = 0x5A; */
}

static void keyboard_add_key_press_to_buffer(uint8_t node, char key)
{
    InterlockedCompareExchange(
        &ezusb_iidx_emu_node_serial_emulation_state[node].keypad_buf,
        convert_keyboard_char(key),
        0);
}

static int ezusb_iidx_emu_node_serial_emu_thread_proc(void *ctx)
{
    log_info("Started magnetic card reader emulation thread");

    /* wait until we are allowed to execute */
    uint8_t init = 0;
    while (init < 2) {
        for (uint8_t i = 0; i < 2; i++) {
            if (ezusb_iidx_emu_node_serial_emulation_state[i].state_current ==
                EMU_STATE_INIT) {
                InterlockedExchange(
                    &ezusb_iidx_emu_node_serial_emulation_state[i]
                         .state_current,
                    EMU_STATE_LOOP);
                init++;
            }
        }

        Sleep(10);
    }

    uint16_t keyboard_state_prev[2] = {0, 0};
    while (true) {
        for (uint8_t node = 0; node < 2; node++) {
            if (!eam_io_poll(node)) {
                log_warning("Polling eamio, node %d failed", node);
                continue;
            }

            /* read card slot sensors */

            uint8_t sensors = eam_io_get_sensor_state(node);
            if (sensors & (1 << EAM_IO_SENSOR_FRONT)) {
                ezusb_iidx_emu_node_serial_emulation_state[node]
                    .card_slot_sensor_front = true;
            } else {
                ezusb_iidx_emu_node_serial_emulation_state[node]
                    .card_slot_sensor_front = false;
            }

            if (sensors & (1 << EAM_IO_SENSOR_BACK)) {
                ezusb_iidx_emu_node_serial_emulation_state[node]
                    .card_slot_sensor_back = true;
            } else {
                ezusb_iidx_emu_node_serial_emulation_state[node]
                    .card_slot_sensor_back = false;
            }

            /* handle keypad */

            uint16_t keypad = eam_io_get_keypad_state(node);
            uint16_t keypad_rise = ~keyboard_state_prev[node] & keypad;

            for (uint8_t i = 0;
                 i < sizeof(ezusb_iidx_emu_node_serial_eamio_mapping);
                 ++i) {
                if (keypad_rise & (1 << i)) {
                    keyboard_add_key_press_to_buffer(
                        node, ezusb_iidx_emu_node_serial_eamio_mapping[i]);
                }
            }

            keyboard_state_prev[node] = keypad;

            /* set card slot state */
            switch (ezusb_iidx_emu_node_serial_emulation_state[node]
                        .ezusb_iidx_emu_node_serial_card_slot_state) {
                case EMU_CARD_SLOT_STATE_CLOSE:
                    eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_CLOSE);
                    break;

                case EMU_CARD_SLOT_STATE_EJECT:
                    eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_EJECT);

                    EnterCriticalSection(
                        &ezusb_iidx_emu_node_serial_emulation_state[node]
                             .card_cs);

                    /* invalidate card id */
                    memset(
                        ezusb_iidx_emu_node_serial_emulation_state[node]
                            .card_id,
                        0,
                        CARD_ID_LEN);

                    LeaveCriticalSection(
                        &ezusb_iidx_emu_node_serial_emulation_state[node]
                             .card_cs);
                    break;

                case EMU_CARD_SLOT_STATE_OPEN:
                    eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_OPEN);
                    break;

                case EMU_CARD_SLOT_STATE_READ:
                    eam_io_card_slot_cmd(node, EAM_IO_CARD_SLOT_CMD_READ);

                    /* read once */
                    if (ezusb_iidx_emu_node_serial_emulation_state[node]
                            .state_current == EMU_STATE_REQ_CARD) {
                        EnterCriticalSection(
                            &ezusb_iidx_emu_node_serial_emulation_state[node]
                                 .card_cs);

                        if (!eam_io_read_card(
                                node,
                                ezusb_iidx_emu_node_serial_emulation_state[node]
                                    .card_id,
                                CARD_ID_LEN)) {
                            memset(
                                ezusb_iidx_emu_node_serial_emulation_state[node]
                                    .card_id,
                                0,
                                CARD_ID_LEN);
                            InterlockedExchange(
                                &ezusb_iidx_emu_node_serial_emulation_state
                                     [node]
                                         .state_current,
                                EMU_STATE_ERROR);
                        } else {
                            InterlockedExchange(
                                &ezusb_iidx_emu_node_serial_emulation_state
                                     [node]
                                         .state_current,
                                EMU_STATE_CARD_READ);
                        }

                        LeaveCriticalSection(
                            &ezusb_iidx_emu_node_serial_emulation_state[node]
                                 .card_cs);
                    }

                    break;

                default:
                    break;
            }
        }

        /* avoid cpu banging */
        Sleep(5);
    }

    return 0;
}
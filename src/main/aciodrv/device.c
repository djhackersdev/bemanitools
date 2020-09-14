#define LOG_MODULE "aciodrv-device"

#include <string.h>

#include "aciodrv/device.h"

#include "aciodrv/port.h"

#include "util/hex.h"
#include "util/log.h"

/* Enable to dump all data to the logger */
//#define AC_IO_MSG_LOG

static uint8_t aciodrv_device_msg_counter = 1;
static uint8_t aciodrv_device_node_count;
static char aviodrv_device_node_products[16][4];

static bool aciodrv_device_init(void)
{
    uint8_t init_seq[1] = {AC_IO_SOF};
    uint8_t read_buff[1] = {0x00};

    /* init/reset the device by sending 0xAA until 0xAA is returned */
    int read = 0;
    do {
        if (aciodrv_port_write(init_seq, sizeof(init_seq)) <= 0) {
            return false;
        }

        read = aciodrv_port_read(read_buff, sizeof(read_buff));
    } while ((read <= 0) || (read_buff[0] != init_seq[0]));

    if (read > 0) {
        log_warning("Obtained SOF, clearing out buffer now");
        /* empty buffer by reading all data */
        while (read > 0) {
            read = aciodrv_port_read(init_seq, sizeof(init_seq));
        }
        log_warning("Cleared buffer, init done");

        return read == 0;
    } else {
        log_warning("Read failure when trying to clear device state");
        return false;
    }
}

#ifdef AC_IO_MSG_LOG
static void
aciodrv_device_log_buffer(const char *msg, const uint8_t *buffer, int length)
{
    char str[4096];

    hex_encode_uc((const void *) buffer, length, str, sizeof(str));
    log_misc("%s, length %d: %s", msg, length, str);
}
#endif

static bool aciodrv_device_send(const uint8_t *buffer, int length)
{
    uint8_t send_buf[512];
    int send_buf_pos = 0;
    uint8_t checksum = 0;

    if (length > sizeof(send_buf)) {
        log_warning("Send buffer overflow");
        return false;
    }

#ifdef AC_IO_MSG_LOG
    aciodrv_device_log_buffer("Send (1)", buffer, length);
#endif

    send_buf[send_buf_pos++] = AC_IO_SOF;

    /* TODO overrun checks */
    for (int i = 0; i < length; i++) {
        if (buffer[i] == AC_IO_SOF || buffer[i] == AC_IO_ESCAPE) {
            send_buf[send_buf_pos++] = AC_IO_ESCAPE;
            send_buf[send_buf_pos++] = ~buffer[i];
        } else {
            send_buf[send_buf_pos++] = buffer[i];
        }

        checksum += buffer[i];
    }

    /* we have to escape the checksum as well! */
    if (checksum == AC_IO_SOF || checksum == AC_IO_ESCAPE) {
        send_buf[send_buf_pos++] = AC_IO_ESCAPE;
        send_buf[send_buf_pos++] = ~checksum;
    } else {
        send_buf[send_buf_pos++] = checksum;
    }

#ifdef AC_IO_MSG_LOG
    aciodrv_device_log_buffer("Send (2)", send_buf, send_buf_pos);
#endif

    if (aciodrv_port_write(send_buf, send_buf_pos) != send_buf_pos) {
        log_warning("Sending data with length %d failed", send_buf_pos);
        return false;
    }

    return true;
}

static int aciodrv_device_receive(uint8_t *buffer, int size)
{
    uint8_t recv_buf[512];
    int recv_size = 0;
    int read = 0;
    uint8_t checksum = 0;
    int result_size = 0;

    /* reading a byte stream, we are getting a varying amount
       of 0xAAs before we get a valid message. */
    recv_buf[0] = AC_IO_SOF;
    do {
        read = aciodrv_port_read(recv_buf, 1);
    } while (recv_buf[0] == AC_IO_SOF);

    if (read > 0) {
        size += 1;

        /* recv_buf[0] is already the first byte of the message.
           now read until nothing's left */
        recv_size++;
        size--;

        /* important: we have to know how much data we expect
           and have to read until we reach the requested amount.
           Because this can be interrupted by 0 reads and we
           need to handle escaping (which relies on an up to
           date recv_buf[recv_size]) we loop until we get a
           non-zero read. */
        while (size > 0) {
            do {
                read = aciodrv_port_read(recv_buf + recv_size, 1);
            } while (read == 0);

            if (read < 0) {
                break;
            }

            /* check for escape byte. these don't count towards the
               size we expect! */
            if (recv_buf[recv_size] == AC_IO_ESCAPE) {
                /* next byte is our real data
                   overwrite escape byte */
                do {
                    read = aciodrv_port_read(recv_buf + recv_size, 1);
                } while (read == 0);

                if (read < 0) {
                    break;
                }

                recv_buf[recv_size] = ~recv_buf[recv_size];
            }

            recv_size += read;
            size -= read;
        }

#ifdef AC_IO_MSG_LOG
        aciodrv_device_log_buffer("Recv (1)", recv_buf, recv_size);
        log_warning("Expected %d got %d", recv_buf[4], recv_size - 6);
#endif

        /* recv_size - 1: omit checksum for checksum calc */
        for (int i = 0; i < recv_size - 1; i++) {
            checksum += recv_buf[i];
            buffer[i] = recv_buf[i];
        }

        result_size = recv_size - 1;

#ifdef AC_IO_MSG_LOG
        aciodrv_device_log_buffer("Recv (2)", buffer, result_size);
#endif

        if (checksum != recv_buf[recv_size - 1]) {
            log_warning(
                "Invalid message checksum: %02X != %02X",
                checksum,
                recv_buf[recv_size - 1]);
            return -1;
        }

        return result_size;
    }

    return -1;
}

static uint8_t aciodrv_device_enum_nodes(void)
{
    struct ac_io_message msg;

    msg.addr = 0x00;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_ASSIGN_ADDRS);
    msg.cmd.nbytes = 1;
    msg.cmd.count = 0;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Enumerating nodes failed");
        return 0;
    }

    log_info("Enumerated %d nodes", msg.cmd.count);

    return msg.cmd.count;
}

static bool aciodrv_device_get_version(uint8_t node_id, char product[4])
{
    struct ac_io_message msg;

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_GET_VERSION);
    msg.cmd.nbytes = 0;

    if (!aciodrv_send_and_recv(
            &msg,
            offsetof(struct ac_io_message, cmd.raw) +
                sizeof(struct ac_io_version))) {
        log_warning("Get version of node %d failed", node_id);
        return false;
    }

    log_info(
        "Node %d: type %d, flag %d, version %d.%d.%d, product %c%c%c%c, "
        "build date: %s %s",
        node_id,
        msg.cmd.version.type,
        msg.cmd.version.flag,
        msg.cmd.version.major,
        msg.cmd.version.minor,
        msg.cmd.version.revision,
        msg.cmd.version.product_code[0],
        msg.cmd.version.product_code[1],
        msg.cmd.version.product_code[2],
        msg.cmd.version.product_code[3],
        msg.cmd.version.date,
        msg.cmd.version.time);

    memcpy(product, msg.cmd.version.product_code, 4);

    return true;
}

static bool aciodrv_device_start_node(uint8_t node_id)
{
    struct ac_io_message msg;

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_START_UP);
    msg.cmd.nbytes = 0;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Starting node %d failed", node_id);
        return false;
    }

    log_info("Started node %d, status: %d", node_id, msg.cmd.status);
    return true;
}

bool aciodrv_device_open(const char *port, int baud)
{
    if (!aciodrv_port_open(port, baud)) {
        return false;
    }

    if (!aciodrv_device_init()) {
        return false;
    }

    aciodrv_device_node_count = aciodrv_device_enum_nodes();
    if (aciodrv_device_node_count == 0) {
        return false;
    }

    for (uint8_t i = 0; i < aciodrv_device_node_count; i++) {
        if (!aciodrv_device_get_version(
                i + 1, aviodrv_device_node_products[i])) {
            return false;
        }
    }

    for (uint8_t i = 0; i < aciodrv_device_node_count; i++) {
        if (!aciodrv_device_start_node(i + 1)) {
            return false;
        }
    }

    return true;
}

uint8_t aciodrv_device_get_node_count(void)
{
    return aciodrv_device_node_count;
}

bool aciodrv_device_get_node_product_ident(uint8_t node_id, char product[4])
{
    if (aciodrv_device_node_count == 0 || node_id > aciodrv_device_node_count) {
        return false;
    }

    memcpy(product, aviodrv_device_node_products[node_id], 4);
    return true;
}

bool aciodrv_send_and_recv(struct ac_io_message *msg, int resp_size)
{
    msg->cmd.seq_no = aciodrv_device_msg_counter++;
    int send_size = offsetof(struct ac_io_message, cmd.raw) + msg->cmd.nbytes;

#ifdef AC_IO_MSG_LOG
    log_info(
        "Beginning send on %d: %04x (%d b)",
        msg->addr,
        msg->cmd.code,
        send_size);
#endif
    if (aciodrv_device_send((uint8_t *) msg, send_size) <= 0) {
        return false;
    }

    uint16_t req_code = msg->cmd.code;

#ifdef AC_IO_MSG_LOG
    log_info("Beginning recv: (%d b)", resp_size);
#endif
    if (aciodrv_device_receive((uint8_t *) msg, resp_size) <= 0) {
        return false;
    }

    if (req_code != msg->cmd.code) {
        log_warning(
            "Received invalid response %04X for request %04X",
            msg->cmd.code,
            req_code);
        return false;
    }

    return true;
}

void aciodrv_device_close(void)
{
    aciodrv_port_close();
}

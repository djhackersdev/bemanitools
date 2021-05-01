#define LOG_MODULE "aciodrv-device"

#include <inttypes.h>
#include <string.h>

#include "aciodrv/device.h"

#include "aciodrv/port.h"

#include "util/hex.h"
#include "util/log.h"
#include "util/mem.h"

/* Enable to dump all data to the logger */
//#define AC_IO_MSG_LOG

#define ACIO_MAX_NODES_PER_PORT 16

struct aciodrv_device_ctx {
    HANDLE fd;
    struct aciodrv_device_node_version node_versions[ACIO_MAX_NODES_PER_PORT];
    uint8_t msg_counter;
    uint8_t node_count;
};

static bool aciodrv_device_init(struct aciodrv_device_ctx *device)
{
    uint8_t reset_seq[525] = {0};
    uint8_t init_seq[1] = {AC_IO_SOF};
    uint8_t read_buff[1] = {0};

    /* init/reset the device by sending 525 NULL bytes */
    log_info("Resetting device");
    if (aciodrv_port_write(device->fd, reset_seq, sizeof(reset_seq)) <= 0) {
        return false;
    }

    /* signal to device for a reset */
    EscapeCommFunction(device->fd, SETBREAK);
    Sleep(1450);
    EscapeCommFunction(device->fd, CLRBREAK);
    Sleep(1200);

    /* ignore the next 100 reads (of whatever length) */
    for (size_t i = 0; i < 100; ++i) {
        aciodrv_port_read(device->fd, reset_seq, sizeof(reset_seq));
    }

    /* wait for reset to finish */
    int read = 0;
    do {
        if (aciodrv_port_write(device->fd, init_seq, sizeof(init_seq)) <= 0) {
            return false;
        }

        read = aciodrv_port_read(device->fd, read_buff, sizeof(read_buff));
    } while ((read <= 0) || (read_buff[0] != init_seq[0]));

    if (read > 0) {
        log_info("Obtained SOF, clearing out buffer now");
        /* empty buffer by reading all data */
        while (read > 0) {
            read = aciodrv_port_read(device->fd, init_seq, sizeof(init_seq));
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
aciodrv_device_log_buffer(struct aciodrv_device_ctx *device, const char *msg, const uint8_t *buffer, int length)
{
    char str[4096];

    hex_encode_uc((const void *) buffer, length, str, sizeof(str));
    log_misc("[%p] %s, length %d: %s", device->fd, msg, length, str);
}
#endif

static bool aciodrv_device_send(struct aciodrv_device_ctx *device, const uint8_t *buffer, int length)
{
    uint8_t send_buf[512];
    int send_buf_pos = 0;
    uint8_t checksum = 0;

    if (length > sizeof(send_buf)) {
        log_warning("[%p] Send buffer overflow", device->fd);
        return false;
    }

#ifdef AC_IO_MSG_LOG
    aciodrv_device_log_buffer("Send (1)", device, buffer, length);
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
    aciodrv_device_log_buffer("Send (2)", device, send_buf, send_buf_pos);
#endif

    if (aciodrv_port_write(device->fd, send_buf, send_buf_pos) != send_buf_pos) {
        log_warning("[%p] Sending data with length %d failed", device->fd, send_buf_pos);
        return false;
    }

    return true;
}

static int aciodrv_device_receive(struct aciodrv_device_ctx *device, uint8_t *buffer, int max_resp_size)
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
        read = aciodrv_port_read(device->fd, recv_buf, 1);
    } while (recv_buf[0] == AC_IO_SOF);


    if (read > 0) {
        /* recv_buf[0] is already the first byte of the message.
           now read until nothing's left */
        recv_size++;

        /* important: we have to know how much data we expect
           and have to read until we reach the requested amount.
           Because this can be interrupted by 0 reads and we
           need to handle escaping (which relies on an up to
           date recv_buf[recv_size]) we loop until we get a
           non-zero read. */
        size_t expected_size = offsetof(struct ac_io_message, cmd.nbytes) + 1;
        while (recv_size < expected_size) {
            do {
                read = aciodrv_port_read(device->fd, recv_buf + recv_size, 1);
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
                    read = aciodrv_port_read(
                        device->fd, recv_buf + recv_size, 1);
                } while (read == 0);

                if (read < 0) {
                    break;
                }

                recv_buf[recv_size] = ~recv_buf[recv_size];
            }

            recv_size += read;

            if (recv_size > offsetof(struct ac_io_message, cmd.nbytes)) {
                // header + data + checksum
                expected_size = offsetof(struct ac_io_message, cmd.raw) + ((struct ac_io_message*)recv_buf)->cmd.nbytes + 1;
            }
        }

#ifdef AC_IO_MSG_LOG
        aciodrv_device_log_buffer("Recv (1)", device, recv_buf, recv_size);
        log_warning("Expected %d got %d", max_resp_size - 6, recv_buf[4]);
#endif

        /* recv_size - 1: omit checksum for checksum calc */
        if ((recv_size - 1) > max_resp_size) {
            log_warning("[%p] Expected %d got %d", device->fd, max_resp_size - 6, recv_buf[4]);
            return -1;
        }
        for (int i = 0; i < recv_size - 1; i++) {
            checksum += recv_buf[i];
            buffer[i] = recv_buf[i];
        }

        result_size = recv_size - 1;

#ifdef AC_IO_MSG_LOG
        aciodrv_device_log_buffer("Recv (2)", device, buffer, result_size);
#endif

        if (checksum != recv_buf[recv_size - 1]) {
            log_warning(
                "[%p] Invalid message checksum: %02X != %02X",
                device->fd,
                checksum,
                recv_buf[recv_size - 1]);
            return -1;
        }

        return result_size;
    }

    return -1;
}

static uint8_t aciodrv_device_enum_nodes(struct aciodrv_device_ctx *device)
{
    struct ac_io_message msg;

    msg.addr = 0x00;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_ASSIGN_ADDRS);
    msg.cmd.nbytes = 1;
    msg.cmd.count = 0;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Enumerating nodes failed");
        return 0;
    }

    log_info("Enumerated %d nodes", msg.cmd.count);

    return msg.cmd.count;
}

static bool aciodrv_device_get_version(struct aciodrv_device_ctx *device, uint8_t node_id, struct aciodrv_device_node_version *version)
{
    struct ac_io_message msg;

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_GET_VERSION);
    msg.cmd.nbytes = 0;

    if (!aciodrv_send_and_recv(
            device,
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

    memcpy(version->product, msg.cmd.version.product_code, ACIO_NODE_PRODUCT_CODE_LEN);
    version->major = msg.cmd.version.major;
    version->minor = msg.cmd.version.minor;
    version->revision = msg.cmd.version.revision;

    return true;
}

static bool aciodrv_device_start_node(struct aciodrv_device_ctx *device, uint8_t node_id)
{
    struct ac_io_message msg;

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_START_UP);
    msg.cmd.nbytes = 0;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Starting node %d failed", node_id);
        return false;
    }

    log_info("Started node %d, status: %d", node_id, msg.cmd.status);
    return true;
}

// deprecated name
struct aciodrv_device_ctx *aciodrv_device_open(const char *port_path, int baud)
{
    return aciodrv_device_open_path(port_path, baud);
}

struct aciodrv_device_ctx *aciodrv_device_open_path(const char *port_path, int baud)
{
    HANDLE port = aciodrv_port_open(port_path, baud);

    if (!port) {
        return NULL;
    }

    struct aciodrv_device_ctx *device = xmalloc(sizeof(struct aciodrv_device_ctx));

    memset(device, 0, sizeof(struct aciodrv_device_ctx));
    device->fd = port;
    device->msg_counter = 1;

    if (!aciodrv_device_init(device)) {
        aciodrv_device_close(device);
        return NULL;
    }

    device->node_count = aciodrv_device_enum_nodes(device);

    if (device->node_count == 0) {
        aciodrv_device_close(device);
        return false;
    }

    for (uint8_t i = 0; i < device->node_count; i++) {
        if (!aciodrv_device_get_version(
                device, i + 1, &device->node_versions[i])) {
            aciodrv_device_close(device);
            return NULL;
        }
    }

    for (uint8_t i = 0; i < device->node_count; i++) {
        if (!aciodrv_device_start_node(device, i + 1)) {
            aciodrv_device_close(device);
            return NULL;
        }
    }

    log_info("Opening ACIO device on [%p]", device->fd);
    return device;
}

uint8_t aciodrv_device_get_node_count(struct aciodrv_device_ctx *device)
{
    return device->node_count;
}

bool aciodrv_device_get_node_product_ident(struct aciodrv_device_ctx *device, uint8_t node_id, char product[ACIO_NODE_PRODUCT_CODE_LEN])
{
    if (device->node_count == 0 || node_id > device->node_count) {
        return false;
    }

    memcpy(product, device->node_versions[node_id].product, ACIO_NODE_PRODUCT_CODE_LEN);
    return true;
}

const struct aciodrv_device_node_version *aciodrv_device_get_node_product_version(struct aciodrv_device_ctx *device, uint8_t node_id)
{
    if (device->node_count == 0 || node_id > device->node_count) {
        return NULL;
    }

    return &device->node_versions[node_id];
}

bool aciodrv_send_and_recv(struct aciodrv_device_ctx *device, struct ac_io_message *msg, int max_resp_size)
{
    msg->cmd.seq_no = device->msg_counter++;
    int send_size = offsetof(struct ac_io_message, cmd.raw) + msg->cmd.nbytes;

#ifdef AC_IO_MSG_LOG
    log_info(
        "[%p] Beginning send on %d: %04x (%d b)",
        device->fd,
        msg->addr,
        msg->cmd.code,
        send_size);
#endif
    if (aciodrv_device_send(device, (uint8_t *) msg, send_size) <= 0) {
        return false;
    }

    uint16_t req_code = msg->cmd.code;

#ifdef AC_IO_MSG_LOG
    log_info("[%p] Beginning recv: (%d b)", device->fd, max_resp_size);
#endif
    if (aciodrv_device_receive(device, (uint8_t *) msg, max_resp_size) <= 0) {
        return false;
    }

    if (req_code != msg->cmd.code) {
        log_warning(
            "[%p] Received invalid response %04X for request %04X",
            device->fd,
            msg->cmd.code,
            req_code);
        return false;
    }

    return true;
}

void aciodrv_device_close(struct aciodrv_device_ctx *device)
{
    log_assert(device);

    log_info("Closing ACIO on [%p]", device->fd);

    aciodrv_port_close(device->fd);

    free(device);
}

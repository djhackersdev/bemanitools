#define LOG_MODULE "aciodrv-icca"

#include <string.h>

#include "aciodrv/device.h"
#include "aciodrv/icca.h"

#include "iface-core/log.h"

static bool aciodrv_icca_queue_loop_start(
    struct aciodrv_device_ctx *device, uint8_t node_id)
{
    struct ac_io_message msg;

    log_assert(device);

    msg.addr = node_id;
    msg.cmd.code = ac_io_u16(AC_IO_ICCA_CMD_QUEUE_LOOP_START);
    msg.cmd.nbytes = 1;
    msg.cmd.status = 0;

    if (!aciodrv_send_and_recv(
            device, &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Starting queue loop failed");
        return false;
    }

    log_info(
        "Started queue loop of node %d, status: %d", node_id, msg.cmd.status);

    return true;
}

bool aciodrv_icca_init(struct aciodrv_device_ctx *device, uint8_t node_id)
{
    log_assert(device);

    if (!aciodrv_icca_queue_loop_start(device, node_id + 1)) {
        return false;
    }

    return true;
}

bool aciodrv_icca_set_state(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    int slot_state,
    struct ac_io_icca_state *state)
{
    struct ac_io_message msg;

    log_assert(device);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_ICCA_CMD_SET_SLOT_STATE);
    msg.cmd.nbytes = 2;
    /* buffer size of data we expect */
    msg.cmd.raw[0] = sizeof(struct ac_io_icca_state);
    msg.cmd.raw[1] = slot_state;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + msg.cmd.raw[0])) {
        log_warning("Setting state of node %d failed", node_id + 1);
        return false;
    }

    if (state != NULL) {
        memcpy(state, msg.cmd.raw, sizeof(struct ac_io_icca_state));
    }

    return true;
}

bool aciodrv_icca_get_state(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    struct ac_io_icca_state *state)
{
    struct ac_io_message msg;

    log_assert(device);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_ICCA_CMD_POLL);
    msg.cmd.nbytes = 1;
    /* buffer size of data we expect */
    msg.cmd.count = sizeof(struct ac_io_icca_state);

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + msg.cmd.count)) {
        log_warning("Getting state of node %d failed", node_id + 1);
        return false;
    }

    if (state != NULL) {
        memcpy(state, msg.cmd.raw, sizeof(struct ac_io_icca_state));
    }

    return true;
}

bool aciodrv_icca_read_card(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    struct ac_io_icca_state *state)
{
    struct ac_io_message msg;

    log_assert(device);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_ICCA_CMD_ENGAGE);
    msg.cmd.nbytes = 1;
    /* buffer size of data we expect */
    msg.cmd.count = sizeof(struct ac_io_icca_state);

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + msg.cmd.count)) {
        log_warning("Reading card of node %d failed", node_id + 1);
        return false;
    }

    if (state != NULL) {
        memcpy(state, msg.cmd.raw, sizeof(struct ac_io_icca_state));
    }

    return true;
}

bool aciodrv_icca_is_slotted(struct aciodrv_device_ctx *device, uint8_t node_id)
{
    const struct aciodrv_device_node_version *version;
    version = aciodrv_device_get_node_product_version(device, node_id);

    // current heuristic is to check if version >= 1.5
    if (version) {
        if (version->major == 1) {
            if (version->minor >= 5) {
                return false;
            }
        }
    }

    return true;
}

bool aciodrv_icca_poll_felica(
    struct aciodrv_device_ctx *device, uint8_t node_id)
{
    struct ac_io_message msg;

    log_assert(device);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_ICCA_CMD_POLL_FELICA);
    msg.cmd.nbytes = 4;
    /* buffer size of data we expect */
    msg.cmd.count = 1;

    // additional data, not sure
    msg.cmd.raw[1] = 0x03;
    msg.cmd.raw[2] = 0xFF;
    msg.cmd.raw[3] = 0xFF;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + msg.cmd.count)) {
        log_warning("Reading card of node %d failed", node_id + 1);
        return false;
    }

    return true;
}

#define LOG_MODULE "aciodrv-panb"

#include <string.h>

#include "aciodrv/device.h"
#include "aciodrv/panb.h"

#include "util/log.h"
#include "util/thread.h"

bool aciodrv_panb_start_auto_input(
    struct aciodrv_device_ctx *device, uint8_t node_id, uint8_t node_count)
{
    log_assert(device);
    struct ac_io_message msg;

    /* only the first node is handling the commands */
    if (node_id != 0) {
        return true;
    }

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_PANB_START_AUTO_INPUT);
    msg.cmd.nbytes = 1;
    msg.cmd.count = node_count;

    if (!aciodrv_send(device, &msg)) {
        log_warning("Starting auto input failed");
        return false;
    }

    log_info("Started auto input for %d nodes", node_count);

    return true;
}

bool aciodrv_panb_recv_poll(
    struct aciodrv_device_ctx *device, struct ac_io_panb_poll_in *poll_in)
{
    log_assert(device);
    struct ac_io_message msg;
    struct ac_io_panb_poll_in *poll_res =
        (struct ac_io_panb_poll_in *) &msg.cmd.raw;

    msg.cmd.code = ac_io_u16(AC_IO_CMD_PANB_POLL_REPLY);
    msg.cmd.nbytes = sizeof(struct ac_io_panb_poll_in);

    if (!aciodrv_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + msg.cmd.nbytes + 1)) {
        log_warning("Getting state failed");
        return false;
    }

    if (poll_in != NULL) {
        memcpy(poll_in, poll_res, sizeof(struct ac_io_panb_poll_in));
    }

    return true;
}

bool aciodrv_panb_send_lamp(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    struct ac_io_panb_poll_out *state)
{
    log_assert(device);
    struct ac_io_message msg;

    /* only the first node is handling the commands */
    if (node_id != 0) {
        return true;
    }

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_PANB_SEND_LAMP);
    msg.cmd.nbytes = 0x54;
    memcpy(&msg.cmd.raw, state, sizeof(struct ac_io_panb_poll_out));

    if (!aciodrv_send(device, &msg)) {
        log_warning("Sending lamp state failed");
        return false;
    }

    return true;
}
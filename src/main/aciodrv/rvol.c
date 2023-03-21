#define LOG_MODULE "aciodrv-rvol"

#include <stdio.h>
#include <string.h>

#include "aciodrv/device.h"

#include "util/log.h"

static bool aciodrv_rvol_change_expand_mode(
    struct aciodrv_device_ctx *device, uint8_t node_id, uint8_t mode)
{
    struct ac_io_message msg;

    log_assert(device);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_RVOL_SET_EXPAND_MODE);
    msg.cmd.nbytes = 1;

    msg.cmd.raw[0] = (mode | (2 * mode)) << 6;

    if (!aciodrv_send_and_recv(
            device, &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("RVOL change expand mode failed %d", node_id);
        return false;
    }

    log_info("I/O expand mode set %d, mode: %d", node_id, mode);

    return true;
}

bool aciodrv_rvol_init(struct aciodrv_device_ctx *device, uint8_t node_id)
{
    log_assert(device);

    if (!aciodrv_rvol_change_expand_mode(device, node_id, 1)) {
        return false;
    }

    return true;
}

bool aciodrv_rvol_poll(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    const struct ac_io_rvol_poll_out *pout,
    struct ac_io_rvol_poll_in *pin)
{
    struct ac_io_message msg;

    log_assert(device);
    log_assert(pout);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_RVOL_POLL);
    msg.cmd.nbytes = sizeof(*pout);
    /* buffer size of data we expect */
    msg.cmd.rvol_poll_out = *pout;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + sizeof(*pin))) {
        log_warning("Polling of node %d failed", node_id + 1);
        return false;
    }

    if (pin != NULL) {
        memcpy(pin, &msg.cmd.rvol_poll_in, sizeof(*pin));
    }

    return true;
}

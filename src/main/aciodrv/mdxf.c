#include "mdxf.h"

#define LOG_MODULE "aciodrv-mdxf"

#include <stdio.h>
#include <string.h>

#include "aciodrv/device.h"

#include "util/log.h"

bool aciodrv_mdxf_init(struct aciodrv_device_ctx *device, uint8_t node_id)
{
    log_assert(device);
    
    return true;

    /*
    struct ac_io_message msg = {0};

    log_info("starting autopoll mdxf node %d", node_id);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_MDXF_AUTO_GET_START);
    msg.cmd.nbytes = 2;

    msg.cmd.raw[0] = 0x80;
    msg.cmd.raw[1] = 0x02;

    if (aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + msg.cmd.nbytes + 1)) {
        return true;
    }

    return false;
    */
}

bool aciodrv_mdxf_poll(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    const struct ac_io_mdxf_poll_in *pin)
{
    struct ac_io_message msg = {0};

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_MDXF_POLL);
    msg.cmd.nbytes = 0;

    aciodrv_send_and_recv(
        device,
        &msg,
        offsetof(struct ac_io_message, cmd.raw) +
            sizeof(struct ac_io_mdxf_poll_in) + 1);

    if (pin != NULL) {
        memcpy(pin, &msg.cmd.mdxf_poll_in, sizeof(*pin));
    }

    return true;
}
#define LOG_MODULE "aciodrv-h44b"

#include <stdio.h>
#include <string.h>

#include "acio/h44b.h"

#include "aciodrv/device.h"

#include "util/log.h"

bool aciodrv_h44b_init(
    struct aciodrv_device_ctx *device,
    uint8_t node_id)
{
    // unlike input devices like KFCA, H44B has no watchdog or special init code
    // requirements - shared ACIO node initialisation is enough
    return true;
}

bool aciodrv_h44b_lights(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    const struct ac_io_h44b_output *lights)
{
    struct ac_io_message msg;

    log_assert(device);
    log_assert(lights);

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_H44B_CMD_SET_OUTPUTS);
    msg.cmd.nbytes = sizeof(*lights);
    msg.cmd.h44b_output = *lights;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Polling of node %d failed", node_id + 1);
        return false;
    }

    return true;
}

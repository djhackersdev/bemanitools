#define LOG_MODULE "bio2drv-bi2a_iidx"

#include "bio2drv/bi2a-iidx.h"

#include <stdio.h>
#include <string.h>

#include "aciodrv/device.h"

#include "util/log.h"

// Must be provided on init command. Actual meaning unknown right now.
// Not providing this will not initialize the IO correctly resulting
// in all inputs and outputs (over sub IO) other than 14 keys to not work.
static const uint8_t _BIO2DR_BI2A_IIDX_INIT_DATA = 0x2D;

static bool bio2drv_bi2a_iidx_init_io(
    struct aciodrv_device_ctx *device,
    uint8_t node_id)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(BIO2_BI2A_CMD_INIT);
    msg.cmd.nbytes = 1;
    msg.cmd.param = _BIO2DR_BI2A_IIDX_INIT_DATA;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Init node failed");
        return 0;
    }

    log_warning("Init of node %d, status: %d", node_id, msg.cmd.status);

    return 1;
}

static bool bio2drv_bi2a_iidx_watchdog_start(
    struct aciodrv_device_ctx *device,
    uint8_t node_id)
{
    // exit early and don't actually call watchdog
    // the watchdog call actually returns different sized packets depending on
    // the state this results in an issue during packet processing (see: #68)
    return true;

    /*
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_KFCA_WATCHDOG);
    msg.cmd.nbytes = 2;
    msg.cmd.nbytes = 2;

    // uint16_t: 6000
    msg.cmd.raw[0] = 23;
    msg.cmd.raw[1] = 112;

    if (!aciodrv_send_and_recv(
        device, &msg, offsetof(struct ac_io_message, cmd.raw) + 2
    )) {
        log_warning("Starting watchdog failed"); return false;
    }

    log_warning("Started watchdog of node %d, status: %d",
        node_id, msg.cmd.status);

    return true;
    */
}

bool bio2drv_bi2a_iidx_init(struct aciodrv_device_ctx *device, uint8_t node_id)
{
    if (!bio2drv_bi2a_iidx_init_io(device, node_id)) {
        return false;
    }

    if (!bio2drv_bi2a_iidx_watchdog_start(device, node_id)) {
        return false;
    }

    return true;
}

bool bio2drv_bi2a_iidx_poll(
    struct aciodrv_device_ctx *device,
    uint8_t node_id,
    const struct bi2a_iidx_state_out *pout,
    struct bi2a_iidx_state_in *pin)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(BIO2_BI2A_CMD_POLL);
    msg.cmd.nbytes = sizeof(*pout);
    /* buffer size of data we expect */
    *(struct bi2a_iidx_state_out *) msg.cmd.raw = *pout;

    if (!aciodrv_send_and_recv(
            device,
            &msg,
            offsetof(struct ac_io_message, cmd.raw) + sizeof(*pin))) {
        log_warning("Polling of node %d failed", node_id + 1);
        return false;
    }

    if (pin != NULL) {
        memcpy(pin, &msg.cmd.raw, sizeof(*pin));
    }

    return true;
}

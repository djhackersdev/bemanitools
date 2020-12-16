#define LOG_MODULE "bio2drv-bi2a_sdvx"

#include "bio2drv/bi2a-sdvx.h"

#include <stdio.h>
#include <string.h>

#include "aciodrv/device.h"

#include "util/log.h"

// this is probably InitIO
static bool bio2drv_bi2a_sdvx_init_io(uint8_t node_id)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(BIO2_BI2A_CMD_INIT);
    msg.cmd.nbytes = 1;
    msg.cmd.count = 0x3B;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Init node failed");
        return 0;
    }

    log_warning("Init of node %d, status: %d", node_id, msg.cmd.status);

    return 1;
}

static bool bio2drv_bi2a_sdvx_watchdog_start(uint8_t node_id)
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
        &msg, offsetof(struct ac_io_message, cmd.raw) + 2
    )) {
        log_warning("Starting watchdog failed"); return false;
    }

    log_warning("Started watchdog of node %d, status: %d",
        node_id, msg.cmd.status);

    return true;
    */
}

bool bio2drv_bi2a_sdvx_amp(
    uint8_t node_id,
    uint8_t unused_1,
    uint8_t unused_2,
    uint8_t left,
    uint8_t right)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_KFCA_AMP_CONTROL);
    msg.cmd.nbytes = 4;

    // the BIO2 DOES NOT MATCH THE KFCA for what these mean
    // 0 and 1 are ignored, 2 and 3 seem to map to left/right for ALL 3 AMPS
    // in the future, maybe use windows APIs and write the volume levels there
    msg.cmd.raw[0] = unused_1;
    msg.cmd.raw[1] = unused_2;
    msg.cmd.raw[2] = left;
    msg.cmd.raw[3] = right;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Setting AMP failed");
        return false;
    }

    log_warning("Started AMP node %d", node_id);

    return true;
}

bool bio2drv_bi2a_sdvx_init(uint8_t node_id)
{
    if (!bio2drv_bi2a_sdvx_init_io(node_id)) {
        return false;
    }

    if (!bio2drv_bi2a_sdvx_watchdog_start(node_id)) {
        return false;
    }

    if (!bio2drv_bi2a_sdvx_amp(node_id, 0, 0, 0, 0)) {
        return false;
    }

    return true;
}

bool bio2drv_bi2a_sdvx_poll(
    uint8_t node_id,
    const struct bi2a_sdvx_state_out *pout,
    struct bi2a_sdvx_state_in *pin)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_KFCA_POLL);
    msg.cmd.nbytes = sizeof(*pout);
    /* buffer size of data we expect */
    *(struct bi2a_sdvx_state_out *) msg.cmd.raw = *pout;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + sizeof(*pin))) {
        log_warning("Polling of node %d failed", node_id + 1);
        return false;
    }

    if (pin != NULL) {
        memcpy(pin, &msg.cmd.raw, sizeof(*pin));
    }

    return true;
}

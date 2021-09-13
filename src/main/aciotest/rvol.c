#include "aciotest/rvol.h"

#include "acio/acio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aciodrv/rvol.h"

static struct ac_io_rvol_poll_out pout;

bool aciotest_rvol_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx)
{
    memset(&pout, 0, sizeof(pout));
    return aciodrv_rvol_init(device, node_id);
}

bool aciotest_rvol_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx)
{
    struct ac_io_rvol_poll_in pin;

    if (!aciodrv_rvol_poll(device, node_id, &pout, &pin)) {
        return false;
    }
    uint8_t selected_light = pin.spinners[0] / 43;

    printf(
        ">>> RVOL %d\n"
        "Press: %3d   %3d   %3d\n"
        "          %3d   %3d\n"
        "\n"
        "Spin:  %3d   %3d   %3d\n"
        "          %3d   %3d\n"
        "\n"
        "Pedal: %3d\n"
        "\n"
        "Editing light #%d R: %d G: %d B: %d\n"
        "(Change light with Spinner 1, values with 3-5)\n",
        node_id,
        pin.buttons_2.spinner_1,
        pin.buttons_3.spinner_3,
        pin.buttons_1.spinner_5,
        pin.buttons_2.spinner_2,
        pin.buttons_1.spinner_4,
        pin.spinners[0],
        pin.spinners[2],
        pin.spinners[4],
        pin.spinners[1],
        pin.spinners[3],
        !pin.buttons_1.pedal,
        selected_light,
        pin.spinners[2] / 2,
        pin.spinners[3] / 2,
        pin.spinners[4] / 2
    );

    memset(&pout, 0, sizeof(pout));

    if (selected_light == 5) {
        pout.title.r = pin.spinners[2] / 2;
        pout.title.g = pin.spinners[3] / 2;
        pout.title.b = pin.spinners[4] / 2;
    } else {
        pout.spinner[selected_light].r = pin.spinners[2] / 2;
        pout.spinner[selected_light].g = pin.spinners[3] / 2;
        pout.spinner[selected_light].b = pin.spinners[4] / 2;
    }

    return true;
}

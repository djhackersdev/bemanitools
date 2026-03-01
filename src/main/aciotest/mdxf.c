#include "aciotest/mdxf.h"

#include "acio/acio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aciodrv/mdxf.h"

#define MDXF_PANEL_FLAGS(var)                                   \
    ((var) & 0b1000 ? 'U' : '-'), ((var) & 0b0100 ? 'D' : '-'), \
        ((var) & 0b0010 ? 'L' : '-'), ((var) & 0b0001 ? 'R' : '-')

bool aciotest_mdxf_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx)
{
    *ctx = malloc(sizeof(uint32_t));
    *((uint32_t *) *ctx) = 0;

    return aciodrv_mdxf_init(device, node_id);
}

bool aciotest_mdxf_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx)
{
    struct ac_io_mdxf_poll_in pin;

    if (!aciodrv_mdxf_poll(device, node_id, &pin)) {
        return false;
    }

    printf(
        ">>> MDXF (DDR) P%d:\n"
        "Foot Up    %c%c%c%c\n"
        "Foot Down  %c%c%c%c\n"
        "Foot Left  %c%c%c%c\n"
        "Foot Right %c%c%c%c\n"
        "\n",
        node_id + 1,
        MDXF_PANEL_FLAGS(pin.panel.up),
        MDXF_PANEL_FLAGS(pin.panel.down),
        MDXF_PANEL_FLAGS(pin.panel.left),
        MDXF_PANEL_FLAGS(pin.panel.right));

    return true;
}

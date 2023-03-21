#include "aciotest/bi2a-sdvx.h"

#include "acio/acio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bio2drv/bi2a-sdvx.h"

bool aciotest_bi2a_sdvx_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx)
{
    *ctx = malloc(sizeof(uint32_t));
    *((uint32_t *) *ctx) = 0;

    return bio2drv_bi2a_sdvx_init(device, node_id);
}

bool aciotest_bi2a_sdvx_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx)
{
    struct bi2a_sdvx_state_in pin;
    struct bi2a_sdvx_state_out pout;

    memset(&pout, 0, sizeof(pout));

    if (!bio2drv_bi2a_sdvx_poll(device, node_id, &pout, &pin)) {
        return false;
    }

    pin.raw[0] = ac_io_u16(pin.raw[0]);
    pin.raw[1] = ac_io_u16(pin.raw[1]);

    printf(
        ">>> BI2A (SDVX) %d:\n"
        "BTN A B C D: %d %d %d %d\n"
        "FX-L R: %d %d\n"
        "VOL L: %d\n"
        "VOL R: %d\n"
        "START COIN TEST SERV REC HP: %d %d %d %d %d %d\n",
        node_id,
        pin.buttons_1.b_a,
        pin.buttons_1.b_b,
        pin.buttons_1.b_c,
        pin.buttons_1.b_d,
        pin.buttons_1.b_fxl,
        pin.buttons_2.b_fxr,
        pin.analogs[0].a_val,
        pin.analogs[1].a_val,
        pin.buttons_1.b_start,
        pin.analogs[0].a_coin,
        pin.analogs[0].a_test,
        pin.analogs[0].a_service,
        pin.buttons_1.b_recorder,
        pin.buttons_1.b_headphone);

    return true;
}

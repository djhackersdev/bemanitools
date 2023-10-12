#include "aciotest/bi2a-iidx.h"

#include "acio/acio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bio2drv/bi2a-iidx.h"

bool aciotest_bi2a_iidx_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx)
{
    *ctx = malloc(sizeof(uint32_t));
    *((uint32_t *) *ctx) = 0;

    return bio2drv_bi2a_iidx_init(device, node_id);
}

bool aciotest_bi2a_iidx_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx)
{
    struct bi2a_iidx_state_in pin;
    struct bi2a_iidx_state_out pout;

    memset(&pout, 0, sizeof(pout));

    if (!bio2drv_bi2a_iidx_poll(device, node_id, &pout, &pin)) {
        return false;
    }

    

    printf(
            "|---------------------------------------|\n"
            "| R  Y  G  B      Neons      B  G  Y  R |\n"
            "| %d  %d  %d  %d        %d        %d  %d  %d  %d |\n"
            "|---------------------------------------|\n"
            "|   NOW PLAYING: %c%c%c%c%c%c%c%c%c              |\n"
            "|---------------------------------------|\n"
            "| Effect %d  S1  S2  S3  S4  S5    Test %d|\n"
            "|StartP1 %d  %02d  %02d  %02d  %02d  %02d StartP2 %d|\n"
            "|   VEFX %d                     Service %d|\n"
            "_________________________________________\n"
            "|   __                             __   |\n"
            "|  /   \\            _             /   \\ |\n"
            "| | %03d|    %d %d %d  |%d|  %d %d %d    | %03d| |\n"
            "|  \\___/   %d %d %d %d |_| %d %d %d %d    \\___/ |\n"
            "|                                       |\n"
            "|---------------------------------------|\n"
            "|---------------------------------------|\n",

            pout.SPOTLIGHT2[0].l_state,
            pout.SPOTLIGHT2[1].l_state,
            pout.SPOTLIGHT2[2].l_state,
            pout.SPOTLIGHT2[3].l_state,
            pout.NEONLAMP.l_state,
            pout.SPOTLIGHT1[0].l_state,
            pout.SPOTLIGHT1[1].l_state,
            pout.SPOTLIGHT1[2].l_state,
            pout.SPOTLIGHT1[3].l_state,

            pout.SEG16[0],
            pout.SEG16[1],
            pout.SEG16[2],
            pout.SEG16[3],
            pout.SEG16[4],
            pout.SEG16[5],
            pout.SEG16[6],
            pout.SEG16[7],
            pout.SEG16[8],

            pin.PANEL.y_effect,
            pin.SYSTEM.v_test,

            pin.PANEL.y_start1,
            pin.SLIDER1.s_val,
            pin.SLIDER2.s_val,
            pin.SLIDER3.s_val,
            pin.SLIDER4.s_val,
            pin.SLIDER5.s_val,
            pin.PANEL.y_start2,

            pin.PANEL.y_vefx,
            pin.SYSTEM.v_service,

            pin.TURNTABLE1,
            pin.P1SW2.b_val,
            pin.P1SW4.b_val,
            pin.P1SW6.b_val,
            pin.SYSTEM.v_coin,
            pin.P2SW2.b_val,
            pin.P2SW4.b_val,
            pin.P2SW6.b_val,
            pin.TURNTABLE2,

            pin.P1SW1.b_val,
            pin.P1SW3.b_val,
            pin.P1SW5.b_val,
            pin.P1SW7.b_val,
            pin.P2SW1.b_val,
            pin.P2SW3.b_val,
            pin.P2SW5.b_val,
            pin.P2SW7.b_val

            );

    return true;
}

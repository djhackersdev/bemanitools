#include "aciotest/panb.h"

#include <stdio.h>
#include <stdlib.h>

#include "aciodrv/panb.h"
#include "aciodrv-proc/panb.h"

struct panb_handler_ctx {
    bool running;
};

bool aciotest_panb_handler_init(struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx)
{
    if (node_id != 0) {
        return true;
    }

    *ctx = malloc(sizeof(struct panb_handler_ctx));
    struct panb_handler_ctx *panb_ctx = (struct panb_handler_ctx*)*ctx;
    panb_ctx->running = true;

    return aciodrv_proc_panb_init(device);
}

bool aciotest_panb_handler_update(struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx)
{
    uint8_t button[AC_IO_PANB_MAX_KEYS];
    struct ac_io_panb_poll_out state;
    struct panb_handler_ctx *panb_ctx = (struct panb_handler_ctx *) ctx;

    if (node_id != 0) {
        return true;
    }
    
    if (!panb_ctx->running) {
        printf(">>> PANB:\nDevice has been closed. Press Ctrl+C to exit.");
        return true;
    }
    
    if (!aciodrv_proc_panb_get_state(button)) {
        return false;
    }

    printf(">>> PANB:\nPress first and last keys to close device\n");

    for (int i=0; i<AC_IO_PANB_MAX_KEYS; i++) {
        printf("%01X ", button[i]);
    }
    printf("\n");

    /* I added a key combo to properly close the module.
     * Leaving the PANB device in autopolling state without ever
     * reading packets from the serial interface eventually
     * makes it fall into a state where it doesn't send anything anymore 
     * and won't even respond to a reset, thus requiring a power cycle.
     */
    if (button[0] && button[AC_IO_PANB_MAX_KEYS-1]) {
        aciodrv_proc_panb_fini(device);
        panb_ctx->running = false;
        return true;
    }

    /* light the panel */
    for (int i=0; i<AC_IO_PANB_MAX_KEYS; i++) {
        state.key[i].green = (button[i])? 0x7F : 0;
        state.key[i].red = 0;
        state.key[i].blue = (button[i])? 0x23 : 0;
    }

    if (!aciodrv_panb_send_lamp(device, node_id, &state)) {
        return false;
    }
    
    return true;
}
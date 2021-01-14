#include "aciotest/icca.h"

#include <stdio.h>
#include <stdlib.h>

#include "aciodrv/icca.h"

bool aciotest_icca_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx)
{
    *ctx = malloc(sizeof(uint32_t));
    *((uint32_t *) *ctx) = 0;

    return aciodrv_icca_init(device, node_id);
}

bool aciotest_icca_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx)
{
    if (*((uint32_t *) ctx) == 0) {
        *((uint32_t *) ctx) = 1;

        /* eject cards that were left in the reader */
        if (!aciodrv_icca_set_state(
                device, node_id, AC_IO_ICCA_SLOT_STATE_EJECT, NULL)) {
            return false;
        }
    }

    struct ac_io_icca_state state;

    if (!aciodrv_icca_get_state(device, node_id, &state)) {
        return false;
    }

    printf(
        ">>> ICCA %d:\n"
        "status_code: %d\n"
        "sensor_state: %d\n"
        "keypad_started: %d\n"
        "card_type: %d\n"
        "UUID: %02X%02X%02X%02X%02X%02X%02X%02X\n"
        "key_state: %04X\n"
        "key_events[0]: %02X\n"
        "key_events[1]: %02X\n",
        node_id,
        state.status_code,
        state.sensor_state,
        state.keypad_started,
        state.card_type,
        state.uid[0],
        state.uid[1],
        state.uid[2],
        state.uid[3],
        state.uid[4],
        state.uid[5],
        state.uid[6],
        state.uid[7],
        state.key_state,
        state.key_events[0],
        state.key_events[1]);

    /* eject card with "empty" key */
    if (state.key_state & AC_IO_ICCA_KEYPAD_MASK_EMPTY) {
        if (!aciodrv_icca_set_state(
                device, node_id, AC_IO_ICCA_SLOT_STATE_EJECT, NULL)) {
            return false;
        }
    }

    /* allow new card to be inserted when slot is clear */
    if (!(state.sensor_state & AC_IO_ICCA_SENSOR_MASK_BACK_ON) &&
        !(state.sensor_state & AC_IO_ICCA_SENSOR_MASK_FRONT_ON)) {
        if (!aciodrv_icca_set_state(
                device, node_id, AC_IO_ICCA_SLOT_STATE_OPEN, NULL)) {
            return false;
        }
    }

    /* lock the card when fully inserted */
    if ((state.sensor_state & AC_IO_ICCA_SENSOR_MASK_BACK_ON) &&
        (state.sensor_state & AC_IO_ICCA_SENSOR_MASK_FRONT_ON)) {
        if (!aciodrv_icca_set_state(
                device, node_id, AC_IO_ICCA_SLOT_STATE_CLOSE, NULL)) {
            return false;
        }

        if (!aciodrv_icca_read_card(device, node_id, NULL)) {
            return false;
        }
    }

    return true;
}
#include <dbt.h>
#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "aciodrv/device.h"
#include "aciodrv/icca.h"

#include "bemanitools/eamio.h"

#include "util/log.h"

static const uint8_t eam_io_keypad_mappings[16] = {EAM_IO_KEYPAD_DECIMAL,
                                                   EAM_IO_KEYPAD_3,
                                                   EAM_IO_KEYPAD_6,
                                                   EAM_IO_KEYPAD_9,
                                                   0xFF,
                                                   0xFF,
                                                   0xFF,
                                                   0xFF,
                                                   EAM_IO_KEYPAD_0,
                                                   EAM_IO_KEYPAD_1,
                                                   EAM_IO_KEYPAD_4,
                                                   EAM_IO_KEYPAD_7,
                                                   EAM_IO_KEYPAD_00,
                                                   EAM_IO_KEYPAD_2,
                                                   EAM_IO_KEYPAD_5,
                                                   EAM_IO_KEYPAD_8};

static struct ac_io_icca_state eam_io_icca_state[2];

void eam_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

bool eam_io_init(
    thread_create_t create, thread_join_t join, thread_destroy_t destroy)
{
    if (!aciodrv_device_open("COM1", 57600)) {
        log_warning("Opening acio device on COM1 failed");
        return false;
    }

    for (uint8_t i = 0; i < 2; i++) {
        if (!aciodrv_icca_init(i)) {
            log_warning("Initializing icca %d failed", i);
            return false;
        }
    }

    return true;
}

void eam_io_fini(void)
{
    aciodrv_device_close();
}

uint16_t eam_io_get_keypad_state(uint8_t unit_no)
{
    uint16_t keypad_result = 0;

    uint16_t keypad = eam_io_icca_state[unit_no].key_state;

    for (uint8_t i = 0; i < sizeof(eam_io_keypad_mappings); ++i) {
        if ((keypad & (1 << i)) && eam_io_keypad_mappings[i] != 0xFF) {
            keypad_result |= (1 << eam_io_keypad_mappings[i]);
        }
    }

    return keypad_result;
}

uint8_t eam_io_get_sensor_state(uint8_t unit_no)
{
    uint8_t sensors = 0;

    if ((eam_io_icca_state[unit_no].sensor_state &
         AC_IO_ICCA_SENSOR_MASK_BACK_ON) > 0) {
        sensors |= (1 << EAM_IO_SENSOR_BACK);
    }
    if ((eam_io_icca_state[unit_no].sensor_state &
         AC_IO_ICCA_SENSOR_MASK_FRONT_ON) > 0) {
        sensors |= (1 << EAM_IO_SENSOR_FRONT);
    }

    return sensors;
}

uint8_t eam_io_read_card(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    memcpy(card_id, eam_io_icca_state[unit_no].uid, nbytes);
    if (card_id[0] == 0xe0 && card_id[1] == 0x04) {
        return EAM_IO_CARD_ISO15696;
    } else {
        return EAM_IO_CARD_FELICA;
    }
}

bool eam_io_card_slot_cmd(uint8_t unit_no, uint8_t cmd)
{
    switch (cmd) {
        case EAM_IO_CARD_SLOT_CMD_CLOSE:
            return aciodrv_icca_set_state(
                unit_no, AC_IO_ICCA_SLOT_STATE_CLOSE, NULL);

        case EAM_IO_CARD_SLOT_CMD_OPEN:
            return aciodrv_icca_set_state(
                unit_no, AC_IO_ICCA_SLOT_STATE_OPEN, NULL);

        case EAM_IO_CARD_SLOT_CMD_EJECT:
            return aciodrv_icca_set_state(
                unit_no, AC_IO_ICCA_SLOT_STATE_EJECT, NULL);

        case EAM_IO_CARD_SLOT_CMD_READ:
            return aciodrv_icca_read_card(unit_no, NULL) &&
                aciodrv_icca_get_state(unit_no, &eam_io_icca_state[unit_no]);

        default:
            break;
    }

    return false;
}

bool eam_io_poll(uint8_t unit_no)
{
    return aciodrv_icca_get_state(unit_no, &eam_io_icca_state[unit_no]);
}

const struct eam_io_config_api *eam_io_get_config_api(void)
{
    return NULL;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, void *ctx)
{
    return TRUE;
}

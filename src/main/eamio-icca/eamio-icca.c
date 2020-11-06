// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <dbt.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "acio/icca.h"

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

// States to map slotted state machine to a slightly simplified
// one that fits the states of the wavepass reader. Probably
// the most important part is correct emulation of the slot
// sensores to ensure the calling backend's states are set
// correctly
enum eam_io_icca_card_state {
    EAM_IO_ICCA_CARD_STATE_IDLE = 0,
    EAM_IO_ICCA_CARD_STATE_PROBE = 1,
};

static struct ac_io_icca_state eam_io_icca_state[2];
static enum eam_io_icca_card_state eam_io_icca_card_state[2];
static bool eam_io_icca_card_data_avail[2];
static uint8_t eam_io_icca_card_uid_buffer[2][8];

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

        eam_io_icca_card_state[i] = EAM_IO_ICCA_CARD_STATE_IDLE;
        eam_io_icca_card_data_avail[i] = false;
    }

    log_info("Initialized icca wavepass");

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

    // when card data is available, signal this by telling the backend "card fully inserted"
    // also, keep card "inserted" as long as reading is in progress
    if (eam_io_icca_card_data_avail[unit_no]) {
        sensors |= (1 << EAM_IO_SENSOR_BACK);
        sensors |= (1 << EAM_IO_SENSOR_FRONT);
    }

    return sensors;
}

uint8_t eam_io_read_card(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    // report back with actual data read from card only if card read command
    // status reports back there is data
    if (eam_io_icca_card_data_avail[unit_no]) {
        memcpy(card_id, eam_io_icca_card_uid_buffer[unit_no], nbytes);

        if (card_id[0] == 0xe0 && card_id[1] == 0x04) {
            return EAM_IO_CARD_ISO15696;
        } else {
            return EAM_IO_CARD_FELICA;
        }
    } else {
        // Avoids garbage data reads for wavepass readers since these need to be
        // polled continuously for data
        memset(card_id, 0, nbytes);

        return EAM_IO_CARD_NONE;
    }
}

bool eam_io_card_slot_cmd(uint8_t unit_no, uint8_t cmd)
{
    switch (cmd) {
        case EAM_IO_CARD_SLOT_CMD_CLOSE:
            eam_io_icca_card_state[unit_no] = EAM_IO_ICCA_CARD_STATE_IDLE;
            return true;

        case EAM_IO_CARD_SLOT_CMD_OPEN:
            eam_io_icca_card_state[unit_no] = EAM_IO_ICCA_CARD_STATE_PROBE;

            return true;

        case EAM_IO_CARD_SLOT_CMD_EJECT:
            eam_io_icca_card_data_avail[unit_no] = false;
            eam_io_icca_card_state[unit_no] = EAM_IO_ICCA_CARD_STATE_IDLE;
            return true;

        case EAM_IO_CARD_SLOT_CMD_READ:
            return true;

        default:
            break;
    }

    return false;
}

bool eam_io_poll(uint8_t unit_no)
{
    // On idle, i.e. the game does not signal any interest in reading cards,
    // just to a standard poll to get the latest status of the readers
    if (eam_io_icca_card_state[unit_no] == EAM_IO_ICCA_CARD_STATE_IDLE) {
        return aciodrv_icca_get_state(unit_no, &eam_io_icca_state[unit_no]);
    } else {
        // When either probing or reading the card, do actual reading by 
        // continuously issueing the read command to either keep probing
        // for potential cards nearby or actually getting data from
        // detected cards back.
        if (aciodrv_icca_read_card(unit_no, NULL) && 
                aciodrv_icca_get_state(unit_no, &eam_io_icca_state[unit_no])) {            

            if (!eam_io_icca_card_data_avail[unit_no] && 
                    eam_io_icca_state[unit_no].status_code == AC_IO_ICCA_STATUS_GOT_UID) {
                eam_io_icca_card_data_avail[unit_no] = true;

                // tmp store read card id because stupid mag readers are sending open and close
                // commands all the time which makes creating a sane state machine impossible
                memcpy(
                    eam_io_icca_card_uid_buffer[unit_no], 
                    eam_io_icca_state[unit_no].uid, 
                    sizeof(eam_io_icca_card_uid_buffer[unit_no]));
            }

            return true;
        }
    }

    return false;
}

const struct eam_io_config_api *eam_io_get_config_api(void)
{
    return NULL;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, void *ctx)
{
    return TRUE;
}

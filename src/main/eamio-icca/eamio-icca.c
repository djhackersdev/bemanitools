// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <dbt.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "aciomgr/manager.h"
#include "aciodrv/icca.h"

#include "bemanitools/eamio.h"

#include "cconfig/cconfig-main.h"
#include "eamio-icca/config-icc.h"

#include "util/log.h"

#define NUMBER_OF_EMULATED_READERS 2

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

static struct ac_io_icca_state eam_io_icca_state[NUMBER_OF_EMULATED_READERS];

static struct aciomgr_port_dispatcher *acio_manager_ctx;

static int32_t icca_node_id[NUMBER_OF_EMULATED_READERS];

void eam_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    /* Pass logger functions on to aciomgr so that it has somewhere to write
       its own log output. */
    aciomgr_set_loggers(misc, info, warning, fatal);

    log_to_external(misc, info, warning, fatal);
}

static bool check_if_icca(int node_id) {
    char product[4];
    aciomgr_get_node_product_ident(acio_manager_ctx, node_id, product);

    if (!memcmp(product, "ICCA", 4)) {
        return true;
    }
    if (!memcmp(product, "ICCB", 4)) {
        return true;
    }
    if (!memcmp(product, "ICCC", 4)) {
        return true;
    }

    return false;
}

bool eam_io_init(
    thread_create_t create, thread_join_t join, thread_destroy_t destroy)
{
    struct cconfig *config;
    struct icc_config config_icc;

    config = cconfig_init();

    eamio_icca_config_icc_init(config);

    if (!cconfig_main_config_init(
            config,
            "--icc-config",
            "eamio-icc.conf",
            "--help",
            "-h",
            "eamio-icca",
            CCONFIG_CMD_USAGE_OUT_STDOUT)) {
        cconfig_finit(config);
        exit(EXIT_FAILURE);
    }

    eamio_icca_config_icc_get(&config_icc, config);

    cconfig_finit(config);

    acio_manager_ctx = aciomgr_port_init(config_icc.port, config_icc.baud);

    if (acio_manager_ctx == NULL) {
        log_warning("Opening acio device on COM1 failed");
        return false;
    }

    struct aciodrv_device_ctx *device = aciomgr_port_checkout(acio_manager_ctx);
    for (uint8_t i = 0; i < NUMBER_OF_EMULATED_READERS; i++) {
        icca_node_id[i] = -1;

        for (uint8_t nid = 0; nid < aciomgr_get_node_count(acio_manager_ctx); ++nid) {
            if (check_if_icca(nid)) {
                bool existing_reader = false;
                for (uint8_t j = 0; j < i; j++) {
                    if (nid == icca_node_id[j]) {
                        existing_reader = true;
                        break;
                    }
                }

                if (existing_reader) {
                    continue;
                }

                icca_node_id[i] = nid;

                if (!aciodrv_icca_init(device, icca_node_id[i])) {
                    log_warning("Initializing icca %d failed", i);

                    // if we have at least 1 valid reader, don't fail
                    // (ex: for games that expect only 1 reader)
                    if (i > 0) {
                        aciomgr_port_checkin(acio_manager_ctx);
                        return false;
                    }
                }

                break;
            }
        }
    }

    aciomgr_port_checkin(acio_manager_ctx);
    return true;
}

void eam_io_fini(void)
{
    aciomgr_port_fini(acio_manager_ctx);
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
    // this node is not setup, just return "success""
    if (icca_node_id[unit_no] == -1) {
        return true;
    }

    struct aciodrv_device_ctx *device = aciomgr_port_checkout(acio_manager_ctx);

    bool response = false;
    switch (cmd) {
        case EAM_IO_CARD_SLOT_CMD_CLOSE:
            response = aciodrv_icca_set_state(
                device, unit_no, AC_IO_ICCA_SLOT_STATE_CLOSE, NULL);

        case EAM_IO_CARD_SLOT_CMD_OPEN:
            response = aciodrv_icca_set_state(
                device, unit_no, AC_IO_ICCA_SLOT_STATE_OPEN, NULL);

        case EAM_IO_CARD_SLOT_CMD_EJECT:
            response = aciodrv_icca_set_state(
                device, unit_no, AC_IO_ICCA_SLOT_STATE_EJECT, NULL);

        case EAM_IO_CARD_SLOT_CMD_READ:
            response = aciodrv_icca_read_card(device, unit_no, NULL) &&
                aciodrv_icca_get_state(
                    device, unit_no, &eam_io_icca_state[unit_no]);

        default:
            break;
    }
    aciomgr_port_checkin(acio_manager_ctx);

    return response;
}

bool eam_io_poll(uint8_t unit_no)
{
    // this node is not setup, just return "success""
    if (icca_node_id[unit_no] == -1) {
        return true;
    }

    bool response = aciodrv_icca_get_state(
        aciomgr_port_checkout(acio_manager_ctx),
        unit_no,
        &eam_io_icca_state[unit_no]);
    aciomgr_port_checkin(acio_manager_ctx);

    return response;
}

const struct eam_io_config_api *eam_io_get_config_api(void)
{
    return NULL;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, void *ctx)
{
    return TRUE;
}

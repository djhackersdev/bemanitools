#define LOG_MODULE "ezusb-iidx-emu-node-sec-plug"

#include <string.h>

#include "ezusb-iidx-emu/node-eeprom.h"
#include "ezusb-iidx-emu/node-security-mem.h"
#include "ezusb-iidx-emu/node-security-plug.h"
#include "ezusb-iidx/secplug-cmd.h"

#include "security/rp.h"
#include "security/rp2.h"
#include "security/util.h"

#include "util/log.h"

static struct security_mcode ezusb_iidx_emu_node_security_plug_boot_version;
static uint32_t ezusb_iidx_emu_node_security_plug_boot_seeds[3];

static struct security_rp_sign_key ezusb_iidx_emu_node_security_plug_black_sign_key;
static struct security_rp_sign_key ezusb_iidx_emu_node_security_plug_white_sign_key;

static struct security_mcode ezusb_iidx_emu_node_security_plug_black_mcode;
static struct security_mcode ezusb_iidx_emu_node_security_plug_white_mcode;

static struct security_id ezusb_iidx_emu_node_security_plug_pcbid;
static struct security_id ezusb_iidx_emu_node_security_plug_eamid;

static enum ezusb_iidx_secplug_dongle_slot
    ezusb_iidx_emu_node_security_plug_active_dongle_slot;
static enum ezusb_iidx_secplug_dongle_memory
    ezusb_iidx_emu_node_security_plug_active_dongle_mem;
static uint8_t ezusb_iidx_emu_node_security_plug_enc_rom_data_seed;

static void ezusb_iidx_emu_node_security_plug_encrypt_rom_data(
    uint8_t *buffer, uint8_t length);

/* ------------------------------------------------------------------------- */
void ezusb_iidx_emu_node_security_plug_set_boot_version(
    const struct security_mcode *boot_version)
{
    char *tmp;

    log_assert(boot_version);

    tmp = security_mcode_to_str(boot_version);

    log_misc("boot version: %s", tmp);
    free(tmp);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_boot_version,
        boot_version,
        sizeof(struct security_mcode));
}

void ezusb_iidx_emu_node_security_plug_set_boot_seeds(const uint32_t *seeds)
{
    log_assert(seeds);

    log_misc("boot seeds: %d %d %d", seeds[0], seeds[1], seeds[2]);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_boot_seeds,
        seeds,
        sizeof(ezusb_iidx_emu_node_security_plug_boot_seeds));
}

void ezusb_iidx_emu_node_security_plug_set_plug_black_sign_key(
    const struct security_rp_sign_key *sign_key)
{
    log_misc("black sign key: %s", sign_key->data);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_black_sign_key,
        sign_key,
        sizeof(struct security_rp_sign_key));
}

void ezusb_iidx_emu_node_security_plug_set_plug_white_sign_key(
    const struct security_rp_sign_key *sign_key)
{
    log_misc("white sign key: %s", sign_key->data);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_white_sign_key,
        sign_key,
        sizeof(struct security_rp_sign_key));
}

void ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
    const struct security_mcode *mcode)
{
    char *tmp;

    log_assert(mcode);

    tmp = security_mcode_to_str(mcode);

    log_misc("black mcode: %s", tmp);
    free(tmp);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_black_mcode,
        mcode,
        sizeof(struct security_mcode));
}

void ezusb_iidx_emu_node_security_plug_set_plug_white_mcode(
    const struct security_mcode *mcode)
{
    char *tmp;

    log_assert(mcode);

    tmp = security_mcode_to_str(mcode);

    log_misc("white mcode: %s", tmp);
    free(tmp);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_white_mcode,
        mcode,
        sizeof(struct security_mcode));
}

void ezusb_iidx_emu_node_security_plug_set_pcbid(
    const struct security_id *pcbid)
{
    char *tmp;

    tmp = security_id_to_str(pcbid, false);

    log_misc("PCBID: %s", tmp);
    free(tmp);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_pcbid,
        pcbid,
        sizeof(struct security_id));
}

void ezusb_iidx_emu_node_security_plug_set_eamid(
    const struct security_id *eamid)
{
    char *tmp;

    tmp = security_id_to_str(eamid, false);

    log_misc("EAMID: %s", tmp);
    free(tmp);

    memcpy(
        &ezusb_iidx_emu_node_security_plug_eamid,
        eamid,
        sizeof(struct security_id));
}

/* ------------------------------------------------------------- */

uint8_t ezusb_iidx_emu_node_security_plug_process_cmd_v1(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_SECPLUG_CMD_V1_READ_ROM:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V1_READ_ROM");
            ezusb_iidx_emu_node_security_plug_active_dongle_mem =
                EZUSB_IIDX_SECPLUG_DONGLE_MEM_ROM;
            ezusb_iidx_emu_node_security_plug_enc_rom_data_seed = cmd_data;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V1_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V1_READ_DATA:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V1_READ_DATA");
            ezusb_iidx_emu_node_security_plug_active_dongle_mem =
                EZUSB_IIDX_SECPLUG_DONGLE_MEM_DATA;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V1_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V1_WRITE_DATA:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V1_WRITE_DATA");
            /* TODO ? */
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V1_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V1_SELECT_BLACK_DONGLE:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V1_SELECT_BLACK_DONGLE");
            ezusb_iidx_emu_node_security_plug_active_dongle_slot =
                EZUSB_IIDX_SECPLUG_DONGLE_SLOT_BLACK;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V1_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V1_SELECT_WHITE_DONGLE:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V1_SELECT_WHITE_DONGLE");
            ezusb_iidx_emu_node_security_plug_active_dongle_slot =
                EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V1_OK;

        default:
            log_warning("Unrecognised security plug v1 command: %02x", cmd_id);
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V1_FAULT;
    }
}

uint8_t ezusb_iidx_emu_node_security_plug_process_cmd_v2(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_SECPLUG_CMD_V2_SEARCH:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_SEARCH");
            ezusb_iidx_emu_node_security_plug_active_dongle_mem =
                EZUSB_IIDX_SECPLUG_DONGLE_MEM_ROM;
            ezusb_iidx_emu_node_security_plug_enc_rom_data_seed = cmd_data;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_SEARCH_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V2_READ_DATA:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_READ_DATA");
            ezusb_iidx_emu_node_security_plug_active_dongle_mem =
                EZUSB_IIDX_SECPLUG_DONGLE_MEM_DATA;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_READ_DATA_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V2_READ_ROM:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_READ_ROM");
            ezusb_iidx_emu_node_security_plug_active_dongle_mem =
                EZUSB_IIDX_SECPLUG_DONGLE_MEM_ROM;
            ezusb_iidx_emu_node_security_plug_enc_rom_data_seed = cmd_data;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_READ_ROM_OK;

        // pop'n music only ever uses slot 2 (white) and slot 4 (black).
        // If the configuration does not match then it will error out.
        // IIDX uses slots 2 and 3 and detects what is plugged into each slot so ordering isn't important.
        // IIDX also reads slot 1's data section but doesn't care what dongle it is?
        case EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_1:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_1");
            ezusb_iidx_emu_node_security_plug_active_dongle_slot =
                EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_SECURITY_SEL_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_2:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_2");
            ezusb_iidx_emu_node_security_plug_active_dongle_slot =
                EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_SECURITY_SEL_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_3:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_3");
            ezusb_iidx_emu_node_security_plug_active_dongle_slot =
                EZUSB_IIDX_SECPLUG_DONGLE_SLOT_BLACK;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_SECURITY_SEL_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_4:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_4");
            ezusb_iidx_emu_node_security_plug_active_dongle_slot =
                EZUSB_IIDX_SECPLUG_DONGLE_SLOT_BLACK;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_SECURITY_SEL_OK;

        case EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_5:
            log_misc("EZUSB_IIDX_SECPLUG_CMD_V2_SELECT_DONGLE_5");
            // Unknown, not encountered during testing
            ezusb_iidx_emu_node_security_plug_active_dongle_slot =
                EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE;
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_SECURITY_SEL_OK;

        default:
            log_warning("Unrecognised security plug v2 command: %02x", cmd_id);
            return EZUSB_IIDX_SECPLUG_CMD_STATUS_V2_FAIL;
    }
}

bool ezusb_iidx_emu_node_security_plug_read_packet_v1(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    char *tmp;

    memset(pkg, 0x00, sizeof(struct ezusb_iidx_msg_bulk_packet));

    if (ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_BLACK &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_ROM) {
        tmp =
            security_id_to_str(&ezusb_iidx_emu_node_security_plug_pcbid, false);

        log_misc("Reading security plug v1 black rom, PCBID %s", tmp);

        free(tmp);

        if (!security_id_verify(&ezusb_iidx_emu_node_security_plug_pcbid)) {
            log_fatal("PCBID verification failed");
            return false;
        }

        pkg->node = 0x11;
        pkg->page = 0x00;

        memcpy(
            pkg->payload,
            &ezusb_iidx_emu_node_security_plug_pcbid,
            sizeof(struct security_id));

        ezusb_iidx_emu_node_security_plug_encrypt_rom_data(pkg->payload, 10);
    } else if (
        ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_BLACK &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_DATA) {
        log_misc("Reading security plug v1 black data");

        pkg->node = 0x12;
        pkg->page = 0x00;

        security_rp_generate_signed_eeprom_data(
            &ezusb_iidx_emu_node_security_plug_boot_version,
            ezusb_iidx_emu_node_security_plug_boot_seeds,
            &ezusb_iidx_emu_node_security_plug_black_mcode,
            &ezusb_iidx_emu_node_security_plug_pcbid,
            (struct security_rp_eeprom *) pkg->payload);

        /* the signed test vector will be compared to the eeprom contents
           write test vector to eeprom to pass checks */
        ezusb_iidx_emu_node_eeprom_write_memory(pkg->payload, 6, 6);
    } else if (
        ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_ROM) {
        tmp =
            security_id_to_str(&ezusb_iidx_emu_node_security_plug_eamid, false);

        log_misc("Reading security plug v1 white rom, EAMID %s", tmp);

        free(tmp);

        if (!security_id_verify(&ezusb_iidx_emu_node_security_plug_eamid)) {
            log_fatal("EAMID verification failed");
            return false;
        }

        pkg->node = 0x11;
        pkg->page = 0x00;

        memcpy(
            pkg->payload,
            &ezusb_iidx_emu_node_security_plug_eamid,
            sizeof(struct security_id));

        ezusb_iidx_emu_node_security_plug_encrypt_rom_data(pkg->payload, 10);
    } else if (
        ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_DATA) {
        log_misc("Reading security plug v1 white data");

        pkg->node = 0x12;
        pkg->page = 0x00;

        security_rp_generate_signed_eeprom_data(
            &ezusb_iidx_emu_node_security_plug_boot_version,
            ezusb_iidx_emu_node_security_plug_boot_seeds,
            &ezusb_iidx_emu_node_security_plug_white_mcode,
            &ezusb_iidx_emu_node_security_plug_eamid,
            (struct security_rp_eeprom *) pkg->payload);

        /* XXX position 0 not verified, guess
           the test vector will be compared against eeprom contents
           write test vector to eeprom to pass checks */
        ezusb_iidx_emu_node_eeprom_write_memory(pkg->payload, 0, 6);
    } else {
        log_warning("Invalid security plug v1 read");
        return false;
    }

    return true;
}

bool ezusb_iidx_emu_node_security_plug_read_packet_v2(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    char *tmp;

    log_misc("Reading security plug v2 packet");

    memset(pkg, 0x00, sizeof(struct ezusb_iidx_msg_bulk_packet));

    if (ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_BLACK &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_ROM) {
        tmp =
            security_id_to_str(&ezusb_iidx_emu_node_security_plug_pcbid, false);

        log_misc("Reading security plug v2 black rom, PCBID %s", tmp);

        free(tmp);

        if (!security_id_verify(&ezusb_iidx_emu_node_security_plug_pcbid)) {
            log_fatal("PCBID verification failed");
            return false;
        }

        pkg->node = 0x11;
        pkg->page = 0x00;

        memcpy(
            pkg->payload,
            &ezusb_iidx_emu_node_security_plug_pcbid,
            sizeof(struct security_id));

        ezusb_iidx_emu_node_security_plug_encrypt_rom_data(pkg->payload, 10);
    } else if (
        ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_BLACK &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_DATA) {
        log_misc("Reading security plug v2 black data");

        pkg->node = 0x12;
        pkg->page = 0x00;

        security_rp2_generate_signed_eeprom_data(
            SECURITY_RP_UTIL_RP_TYPE_BLACK,
            &ezusb_iidx_emu_node_security_plug_black_sign_key,
            &ezusb_iidx_emu_node_security_plug_black_mcode,
            &ezusb_iidx_emu_node_security_plug_pcbid,
            (struct security_rp2_eeprom *) pkg->payload);
    } else if (
        ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_ROM) {
        tmp =
            security_id_to_str(&ezusb_iidx_emu_node_security_plug_eamid, false);

        log_misc("Reading security plug v2 white rom, EAMID %s", tmp);

        free(tmp);

        if (!security_id_verify(&ezusb_iidx_emu_node_security_plug_eamid)) {
            log_fatal("EAMID verification failed");
            return false;
        }

        pkg->node = 0x11;
        pkg->page = 0x00;

        memcpy(
            pkg->payload,
            &ezusb_iidx_emu_node_security_plug_eamid,
            sizeof(struct security_id));

        ezusb_iidx_emu_node_security_plug_encrypt_rom_data(pkg->payload, 10);
    } else if (
        ezusb_iidx_emu_node_security_plug_active_dongle_slot ==
            EZUSB_IIDX_SECPLUG_DONGLE_SLOT_WHITE &&
        ezusb_iidx_emu_node_security_plug_active_dongle_mem ==
            EZUSB_IIDX_SECPLUG_DONGLE_MEM_DATA) {
        log_misc("Reading security plug v2 white data");

        pkg->node = 0x12;
        pkg->page = 0x00;

        security_rp2_generate_signed_eeprom_data(
            SECURITY_RP_UTIL_RP_TYPE_WHITE,
            &ezusb_iidx_emu_node_security_plug_white_sign_key,
            &ezusb_iidx_emu_node_security_plug_white_mcode,
            &ezusb_iidx_emu_node_security_plug_eamid,
            (struct security_rp2_eeprom *) pkg->payload);
    }

    return true;
}

bool ezusb_iidx_emu_node_security_plug_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_misc("Writing security plug packet");
    return true;
}

/* ------------------------------------------------------------- */

static void ezusb_iidx_emu_node_security_plug_encrypt_rom_data(
    uint8_t *buffer, uint8_t length)
{
    uint8_t data;

    for (int i = 0; i < length; i++) {
        data = ezusb_iidx_emu_node_security_mem_read_memory(
            (ezusb_iidx_emu_node_security_plug_enc_rom_data_seed + i) & 0xFF);
        /*
        log_misc("Encrypting %02X, seed %02X with sec mem lookup %02X",
            buffer[i], ezusb_iidx_emu_node_security_plug_enc_rom_data_seed,
            data);
        */
        buffer[i] ^= data;
    }
}
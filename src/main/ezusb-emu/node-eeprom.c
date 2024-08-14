#define LOG_MODULE "ezusb-emu-node-eeprom"

#include <string.h>

#include "core/log.h"

#include "ezusb-emu/node-eeprom.h"

#include "ezusb-iidx/eeprom-cmd.h"

/* not verified, but we got calls with 3 pages only so far */
#define EEPROM_NPAGES 3

static uint8_t ezusb_iidx_emu_node_eeprom_read_page_pos;
static uint8_t ezusb_iidx_emu_node_eeprom_mem[EZUSB_PAGESIZE * EEPROM_NPAGES];

void ezusb_iidx_emu_node_eeprom_init(void)
{
    ezusb_iidx_emu_node_eeprom_read_page_pos = 0;
    memset(
        ezusb_iidx_emu_node_eeprom_mem,
        0xFF,
        sizeof(ezusb_iidx_emu_node_eeprom_mem));
}

uint8_t ezusb_iidx_emu_node_eeprom_process_cmd_v1(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_EEPROM_CMD_READ:
            log_misc("EZUSB_EEPROM_V1_CMD_READ");
            ezusb_iidx_emu_node_eeprom_read_page_pos = 0;
            return EZUSB_IIDX_EEPROM_CMD_STATUS_V1_READ_OK;

        case EZUSB_IIDX_EEPROM_CMD_WRITE:
            log_misc("EZUSB_EEPROM_V1_CMD_WRITE");
            return EZUSB_IIDX_EEPROM_CMD_STATUS_V2_WRITE_OK;

        default:
            log_warning("Unrecognised eeprom v1 command: %02x", cmd_id);
            return EZUSB_IIDX_EEPROM_CMD_STATUS_V1_FAULT;
    }
}

uint8_t ezusb_iidx_emu_node_eeprom_process_cmd_v2(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_EEPROM_CMD_READ:
            log_misc("EZUSB_EEPROM_V2_CMD_READ");
            ezusb_iidx_emu_node_eeprom_read_page_pos = 0;
            return EZUSB_IIDX_EEPROM_CMD_STATUS_V2_READ_OK;

        case EZUSB_IIDX_EEPROM_CMD_WRITE:
            log_misc("EZUSB_EEPROM_V2_CMD_WRITE");
            return EZUSB_IIDX_EEPROM_CMD_STATUS_V2_WRITE_OK;

        default:
            log_warning("Unrecognised eeprom v2 command: %02x", cmd_id);
            return EZUSB_IIDX_EEPROM_CMD_STATUS_V2_FAULT;
    }
}

bool ezusb_iidx_emu_node_eeprom_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_misc(
        "Reading EEPROM page 0x%02X", ezusb_iidx_emu_node_eeprom_read_page_pos);

    if (ezusb_iidx_emu_node_eeprom_read_page_pos >= EEPROM_NPAGES) {
        log_warning("Reading EEPROM ezusb_iidx_emu_node_eeprom_mem overrun");
        return false;
    }

    /* has to be 0x22 to get accepted */
    pkg->node = 0x22;
    pkg->page = ezusb_iidx_emu_node_eeprom_read_page_pos;
    memset(pkg->payload, 0, EZUSB_PAGESIZE);
    memcpy(
        pkg->payload,
        ezusb_iidx_emu_node_eeprom_mem +
            ezusb_iidx_emu_node_eeprom_read_page_pos * EZUSB_PAGESIZE,
        EZUSB_PAGESIZE);

    ezusb_iidx_emu_node_eeprom_read_page_pos++;
    return true;
}

bool ezusb_iidx_emu_node_eeprom_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_misc("Writing EEPROM page 0x%02X", pkg->page);

    if (pkg->page >= EEPROM_NPAGES) {
        log_warning("Writing EEPROM ezusb_iidx_emu_node_eeprom_mem overrun");
        return false;
    }

    memcpy(
        ezusb_iidx_emu_node_eeprom_mem + pkg->page * EZUSB_PAGESIZE,
        pkg->payload,
        EZUSB_PAGESIZE);
    return true;
}

/* ------------------------------------------------------------------------- */

size_t ezusb_iidx_emu_node_eeprom_write_memory(
    const uint8_t *buffer, size_t offset, size_t length)
{
    if (offset + length > sizeof(ezusb_iidx_emu_node_eeprom_mem)) {
        log_warning(
            "Writing eeprom ezusb_iidx_emu_node_eeprom_mem overrun, "
            "truncated");
        length = sizeof(ezusb_iidx_emu_node_eeprom_mem) - offset;
    }

    memcpy(ezusb_iidx_emu_node_eeprom_mem + offset, buffer, length);
    return length;
}
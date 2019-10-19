#ifndef EZUSB_IIDX_EMU_NODE_EEPROM_H
#define EZUSB_IIDX_EMU_NODE_EEPROM_H

#include "ezusb-iidx-emu/node.h"

void ezusb_iidx_emu_node_eeprom_init(void);
/* Used on 9th to DistorteD */
uint8_t ezusb_iidx_emu_node_eeprom_process_cmd_v1(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

/* Used on Gold to Sirius */
uint8_t ezusb_iidx_emu_node_eeprom_process_cmd_v2(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

bool ezusb_iidx_emu_node_eeprom_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg);

bool ezusb_iidx_emu_node_eeprom_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg);

/* Used by the security plug to write security related data */
size_t ezusb_iidx_emu_node_eeprom_write_memory(
    const uint8_t *buffer, size_t offset, size_t length);

#endif
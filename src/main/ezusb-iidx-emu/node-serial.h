#ifndef EZUSB_IIDX_EMU_NODE_SERIAL_H
#define EZUSB_IIDX_EMU_NODE_SERIAL_H

#include "ezusb-iidx-emu/node.h"

void ezusb_iidx_emu_node_serial_init(void);

uint8_t ezusb_iidx_emu_node_serial_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

bool ezusb_iidx_emu_node_serial_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg);

bool ezusb_iidx_emu_node_serial_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg);

/**
 * Check if the uart read buffer is busy (refer to ezusb interrupt read)
 */
bool ezusb_iidx_emu_node_serial_read_buffer_busy(void);

/**
 * Check if the uart write buffer is busy (refer to ezusb interrupt read)
 */
bool ezusb_iidx_emu_node_serial_write_buffer_busy(void);

/**
 * Set a few attributes to emulate magnetic cards for different game versions
 * @param card_type Type of the card (0 - 4)
 * @param used_Card Used card with data on it or empty card. If you don't
 *                  know what you are doing set this to true.
 * @param card_version Version of the card to emulate (mcode of the game
 *                     that supports mag cards)
 */
void ezusb_iidx_emu_node_serial_set_card_attributes(
    uint8_t card_type, bool used_card, const char *card_version);

#endif
#ifndef EZUSB_IIDX_EMU_NODE_WDT_H
#define EZUSB_IIDX_EMU_NODE_WDT_H

#include "ezusb-iidx-emu/node.h"

uint8_t ezusb_iidx_emu_node_wdt_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

bool ezusb_iidx_emu_node_wdt_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg);

bool ezusb_iidx_emu_node_wdt_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg);

#endif
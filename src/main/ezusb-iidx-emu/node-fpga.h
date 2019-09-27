#ifndef EZUSB_IIDX_EMU_NODE_FPGA_H
#define EZUSB_IIDX_EMU_NODE_FPGA_H

#include "ezusb-iidx-emu/node.h"

/* Used on 9th to DistorteD */
uint8_t ezusb_iidx_emu_node_fpga_v1_process_cmd(uint8_t cmd_id, uint8_t cmd_data,
        uint8_t cmd_data2);

/* Used on Gold onwards */
uint8_t ezusb_iidx_emu_node_fpga_v2_process_cmd(uint8_t cmd_id, uint8_t cmd_data,
        uint8_t cmd_data2);

bool ezusb_iidx_emu_node_fpga_read_packet(struct ezusb_iidx_msg_bulk_packet* pkg);

bool ezusb_iidx_emu_node_fpga_write_packet(const struct ezusb_iidx_msg_bulk_packet* pkg);

#endif
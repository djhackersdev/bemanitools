#ifndef EZUSB_IIDX_EMU_NODE_SECURITY_MEM_H
#define EZUSB_IIDX_EMU_NODE_SECURITY_MEM_H

#include "ezusb-iidx-emu/node.h"

void ezusb_iidx_emu_node_security_mem_init(void);

/* Used on 9th to DistorteD */
uint8_t ezusb_iidx_emu_node_security_mem_v1_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

/* Used on Gold to Sirius */
uint8_t ezusb_iidx_emu_node_security_mem_v2_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

bool ezusb_iidx_emu_node_security_mem_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg);

bool ezusb_iidx_emu_node_security_mem_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg);

/* Used by the security plug to read from security memory to encrypt data
   sent to the host */
uint8_t ezusb_iidx_emu_node_security_mem_read_memory(uint32_t pos);

#endif
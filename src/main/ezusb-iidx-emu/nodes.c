/*
 * This file contains the ezusb nodes that appeared on
 * all IIDX games so far.
 */

#include "ezusb-iidx/msg.h"

#include "ezusb-iidx-emu/node-16seg.h"
#include "ezusb-iidx-emu/node-fpga.h"
#include "ezusb-iidx-emu/node-serial.h"
#include "ezusb-iidx-emu/nodes.h"

/* All IIDX games */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_16seg = {
    .node_id = EZUSB_IIDX_MSG_NODE_16SEG,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_16seg_process_cmd,
    .read_packet = ezusb_iidx_emu_node_16seg_read_packet,
    .write_packet = ezusb_iidx_emu_node_16seg_write_packet};

/* Used on 9th to DistorteD */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_fpga_v1 = {
    .node_id = EZUSB_IIDX_MSG_NODE_FPGA_V1,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_fpga_v1_process_cmd,
    .read_packet = ezusb_iidx_emu_node_fpga_read_packet,
    .write_packet = ezusb_iidx_emu_node_fpga_write_packet};

/* Used on Gold onwards */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_fpga_v2 = {
    .node_id = EZUSB_IIDX_MSG_NODE_FPGA_V2,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_fpga_v2_process_cmd,
    .read_packet = ezusb_iidx_emu_node_fpga_read_packet,
    .write_packet = ezusb_iidx_emu_node_fpga_write_packet};

/* Used on 9th to HappySky for magnetic readers */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_serial = {
    .node_id = EZUSB_IIDX_MSG_NODE_SERIAL,
    .init_node = ezusb_iidx_emu_node_serial_init,
    .process_cmd = ezusb_iidx_emu_node_serial_process_cmd,
    .read_packet = ezusb_iidx_emu_node_serial_read_packet,
    .write_packet = ezusb_iidx_emu_node_serial_write_packet};

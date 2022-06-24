/*
 * This file contains the ezusb nodes that appeared on
 * all IIDX games so far.
 */

#include "ezusb-iidx/msg.h"

#include "ezusb-iidx-16seg-emu/node-16seg.h"

/* All IIDX games */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_16seg = {
    .node_id = EZUSB_IIDX_MSG_NODE_16SEG,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_16seg_process_cmd,
    .read_packet = ezusb_iidx_emu_node_16seg_read_packet,
    .write_packet = ezusb_iidx_emu_node_16seg_write_packet};

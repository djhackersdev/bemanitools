#ifndef EZUSB_IIDX_EMU_NODE_H
#define EZUSB_IIDX_EMU_NODE_H

#include <stdint.h>
#include <stdbool.h>

#include "ezusb-iidx/msg.h"

/**
 * Interface for an ezusb node
 */
struct ezusb_iidx_emu_node {
    /* Addressable node id by the ezusb device */
    const uint8_t node_id;

    /* "Constructor" like init call. Called when node is added */
    void (*init_node)(void);

    /* Process an incoming command detrmined for this node */
    uint8_t (*process_cmd)(uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

    /* Bulk read data from the node */
    bool (*read_packet)(struct ezusb_iidx_msg_bulk_packet* pkg);

    /* Bulk write data to the node */
    bool (*write_packet)(const struct ezusb_iidx_msg_bulk_packet* pkg);
};

#endif
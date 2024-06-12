#define LOG_MODULE "ezusb-iidx-emu-node-16seg"

#include "ezusb-iidx-16seg-emu/node-16seg.h"

#include <string.h>

#include "iface-core/log.h"
#include "iface-io/iidx.h"

#include "ezusb-iidx/seg16-cmd.h"

uint8_t ezusb_iidx_emu_node_16seg_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_16SEG_CMD_WRITE:
            return EZUSB_IIDX_16SEG_CMD_STATUS_OK;

        default:
            log_warning("Unknown 16seg node command: %02x", cmd_id);
            return EZUSB_IIDX_16SEG_CMD_STATUS_FAULT;
    }
}

bool ezusb_iidx_emu_node_16seg_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_fatal("Read packet not supported on 16seg node");
    return false;
}

bool ezusb_iidx_emu_node_16seg_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    char _16seg[10];

    memcpy(_16seg, pkg->payload, 9);
    _16seg[9] = '\0';

    return bt_io_iidx_ep3_16seg_send(_16seg);
}
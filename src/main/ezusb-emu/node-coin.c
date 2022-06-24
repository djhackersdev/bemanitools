#define LOG_MODULE "ezusb-emu-node-coin"

#include "ezusb-emu/node-coin.h"

#include "ezusb-iidx/coin-cmd.h"

#include "util/log.h"

static uint8_t ezusb_iidx_emu_node_coin_mode = 0;

uint8_t ezusb_iidx_emu_node_coin_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_COIN_CMD_SET_COIN_MODE_1:
            ezusb_iidx_emu_node_coin_mode = 0;
            return EZUSB_IIDX_COIN_CMD_STATUS_OK;

        case EZUSB_IIDX_COIN_CMD_SET_COIN_MODE_2:
            ezusb_iidx_emu_node_coin_mode = 1;
            return EZUSB_IIDX_COIN_CMD_STATUS_OK;

        default:
            log_warning("Unknown coin node command: %02x", cmd_id);
            return EZUSB_IIDX_COIN_CMD_STATUS_FAULT;
    }
}

bool ezusb_iidx_emu_node_coin_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_fatal("Read packet not supported on coin node");
    return false;
}

bool ezusb_iidx_emu_node_coin_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_fatal("Write packet not supported on coin node");
    return false;
}

uint8_t ezusb_iidx_emu_node_coin_get_mode(void)
{
    return ezusb_iidx_emu_node_coin_mode;
}
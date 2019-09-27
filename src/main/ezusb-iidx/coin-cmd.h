#ifndef EZUSB_IIDX_COIN_CMD_H
#define EZUSB_IIDX_COIN_CMD_H

enum ezusb_iidx_coin_command {
    /* set when "playing" the game */
    EZUSB_IIDX_COIN_CMD_SET_COIN_MODE_1 = 0x01,
    /* set when in service menu */
    EZUSB_IIDX_COIN_CMD_SET_COIN_MODE_2 = 0x02
};

enum ezusb_iidx_coin_command_status {
    EZUSB_IIDX_COIN_CMD_STATUS_OK = 0x00,
    EZUSB_IIDX_COIN_CMD_STATUS_FAULT = 0xFE
};

#endif
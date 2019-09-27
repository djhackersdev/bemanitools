#ifndef EZUSB_IIDX_SEG16_CMD_H
#define EZUSB_IIDX_SEG16_CMD_H

enum ezusb_iidx_16seg_command {
    EZUSB_IIDX_16SEG_CMD_WRITE = 0x03,
};

enum ezusb_iidx_16seg_command_status {
    EZUSB_IIDX_16SEG_CMD_STATUS_OK = 0x00,
    EZUSB_IIDX_16SEG_CMD_STATUS_FAULT = 0xFE
};


#endif
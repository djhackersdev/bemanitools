#ifndef EZUSB_IIDX_SECMEM_CMD_H
#define EZUSB_IIDX_SECMEM_CMD_H

enum ezusb_iidx_secmem_command {
    EZUSB_IIDX_SECMEM_CMD_WRITE = 0x00,
};

enum ezusb_iidx_secmem_command_status_v1 {
    EZUSB_IIDX_SECMEM_CMD_STATUS_V1_WRITE_OK = 0x60,
    EZUSB_IIDX_SECMEM_CMD_STATUS_V1_FAULT = 0xFE,
};

enum ezusb_iidx_secmem_command_status_v2 {
    EZUSB_IIDX_SECMEM_CMD_STATUS_V2_WRITE_OK = 0x11,
    EZUSB_IIDX_SECMEM_CMD_STATUS_V2_FAULT = 0xFE,
};

#endif
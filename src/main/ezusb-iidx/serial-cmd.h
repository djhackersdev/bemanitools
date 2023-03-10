#ifndef EZUSB_IIDX_SERIAL_CMD_H
#define EZUSB_IIDX_SERIAL_CMD_H

enum ezusb_iidx_serial_command {
    EZUSB_IIDX_SERIAL_CMD_READ_BUFFER = 0x02,
    EZUSB_IIDX_SERIAL_CMD_WRITE_BUFFER = 0x03,
    EZUSB_IIDX_SERIAL_CMD_CLEAR_READ_BUFFER = 0x04,
    EZUSB_IIDX_SERIAL_CMD_CLEAR_WRITE_BUFFER = 0x05
};

enum ezusb_iidx_serial_command_status {
    EZUSB_IIDX_SERIAL_CMD_STATUS_OK = 0x00,
    EZUSB_IIDX_SERIAL_CMD_STATUS_FAULT = 0xFE
};

#endif
#ifndef EZUSB_IIDX_SRAM_CMD_H
#define EZUSB_IIDX_SRAM_CMD_H

enum ezusb_iidx_sram_command {
    EZUSB_IIDX_SRAM_CMD_READ = 0x02,
    EZUSB_IIDX_SRAM_CMD_WRITE = 0x03,
    EZUSB_IIDX_SRAM_CMD_DONE = 0x04
};

#endif
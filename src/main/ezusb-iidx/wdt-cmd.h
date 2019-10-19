#ifndef EZUSB_IIDX_WDT_CMD_H
#define EZUSB_IIDX_WDT_CMD_H

enum ezusb_iidx_wdt_command { EZUSB_IIDX_WDT_CMD_INIT = 0x3C };

enum ezusb_iidx_wdt_command_status {
    EZUSB_IIDX_WDT_CMD_STATUS_OK = 0x00,
    EZUSB_IIDX_WDT_CMD_STATUS_FAULT = 0xFE
};

#endif
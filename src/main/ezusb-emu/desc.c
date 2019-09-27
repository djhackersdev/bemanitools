#include "ezusb-emu/desc.h"

const struct ezusb_emu_desc_device ezusb_emu_desc_device = {
    .setupapi = {
        .device_guid = {
            0xAE18AA60,
            0x7F6A,
            0x11D4,
            { 0x97, 0xDD, 0x00, 0x01, 0x02, 0x29, 0xB9, 0x59 }
        },
        .device_desc = "Cypress EZ-USB (2235) - EEPROM missing",
        .device_path = "\\\\.\\Ezusb-0"
    },
    .vid = 0x0547,
    .pid = 0x2235,
};
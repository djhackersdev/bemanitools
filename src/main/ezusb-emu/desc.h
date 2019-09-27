#ifndef EZUSB_EMU_DESC_H
#define EZUSB_EMU_DESC_H

#include <stdint.h>

#include "hooklib/setupapi.h"

struct ezusb_emu_desc_device {
    struct hook_setupapi_data setupapi;
    uint16_t vid;
    uint16_t pid;
};

/* C02 IO board */
extern const struct ezusb_emu_desc_device ezusb_emu_desc_device;

#endif

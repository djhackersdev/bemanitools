#ifndef AC_IO_EMU_ICCB_H
#define AC_IO_EMU_ICCB_H

#include <stdbool.h>
#include <stdint.h>

#include "acioemu/emu.h"

struct ac_io_emu_iccb {
    struct ac_io_emu *emu;
    uint8_t unit_no;
    bool fault;
    bool last_sensor;
    uint8_t uid[8];
    uint8_t card_result;
};

void ac_io_emu_iccb_init(
        struct ac_io_emu_iccb *iccb,
        struct ac_io_emu *emu,
        uint8_t unit_no);

void ac_io_emu_iccb_dispatch_request(
        struct ac_io_emu_iccb *iccb,
        const struct ac_io_message *req);

#endif

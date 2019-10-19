#ifndef AC_IO_EMU_H44B_H
#define AC_IO_EMU_H44B_H

#include <stdbool.h>
#include <stdint.h>

#include "acioemu/emu.h"

struct ac_io_emu_h44b {
    struct ac_io_emu *emu;
    uint8_t unit_no;
    // TODO
};

void acioemu_h44b_init(void);

void ac_io_emu_h44b_init(
    struct ac_io_emu_h44b *h44b, struct ac_io_emu *emu, uint8_t unit_no);

void ac_io_emu_h44b_dispatch_request(
    struct ac_io_emu_h44b *h44b, const struct ac_io_message *req);

#endif

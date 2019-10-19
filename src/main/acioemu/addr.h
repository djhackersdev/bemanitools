#ifndef AC_IO_EMU_ADDR_H
#define AC_IO_EMU_ADDR_H

#include "acio/acio.h"

#include "acioemu/emu.h"

#include <stdint.h>

void ac_io_emu_cmd_assign_addrs(
    struct ac_io_emu *emu, const struct ac_io_message *req, uint8_t node_count);

#endif

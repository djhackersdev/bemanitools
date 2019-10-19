#ifndef AC_IO_EMU_HDXS_H
#define AC_IO_EMU_HDXS_H

#include "acio/acio.h"

#include "acioemu/emu.h"

struct ac_io_emu_hdxs {
    struct ac_io_emu *emu;
    // TODO ops vtbl
};

void ac_io_emu_hdxs_init(struct ac_io_emu_hdxs *hdxs, struct ac_io_emu *emu);

void ac_io_emu_hdxs_dispatch_request(
    struct ac_io_emu_hdxs *hdxs, const struct ac_io_message *req);

#endif

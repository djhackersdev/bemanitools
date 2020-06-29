#ifndef AC_IO_EMU_HDXS_H
#define AC_IO_EMU_HDXS_H

#include "acio/acio.h"

#include "acioemu/emu.h"

struct ac_io_emu_hdxs;
struct ac_io_message;

typedef void (*acio_hdxs_dispatcher)(
    struct ac_io_emu_hdxs *emu, const struct ac_io_message *req);

struct ac_io_emu_hdxs {
    struct ac_io_emu *emu;
    acio_hdxs_dispatcher lights_dispatcher;
};

void ac_io_emu_hdxs_init(
    struct ac_io_emu_hdxs *hdxs,
    struct ac_io_emu *emu,
    acio_hdxs_dispatcher lights_dispatcher);

void ac_io_emu_hdxs_dispatch_request(
    struct ac_io_emu_hdxs *hdxs, const struct ac_io_message *req);

#endif

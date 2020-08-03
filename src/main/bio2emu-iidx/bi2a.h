#ifndef BIO2EMU_IIDX_BI2A_H
#define BIO2EMU_IIDX_BI2A_H

#include "bio2/bi2a-iidx.h"

#include "bio2emu/emu.h"

void bio2_emu_bi2a_init(struct bio2emu_port *in, bool disable_poll_limiter);
void bio2_emu_bi2a_dispatch_request(
    struct bio2emu_port *bio2port, const struct ac_io_message *req);

#endif

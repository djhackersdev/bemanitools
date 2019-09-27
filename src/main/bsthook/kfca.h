#ifndef BSTHOOK_KFCA_H
#define BSTHOOK_KFCA_H

#include "acio/acio.h"

#include "acioemu/emu.h"

void kfca_init(struct ac_io_emu *in);
void kfca_dispatch_request(const struct ac_io_message *req);

#endif

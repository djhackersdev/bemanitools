#ifndef SDVXHOOK_KFCA_H
#define SDVXHOOK_KFCA_H

#include "acio/acio.h"

#include "acioemu/emu.h"

void kfca_init(struct ac_io_emu *emu);
void kfca_dispatch_request(const struct ac_io_message *req);

#endif

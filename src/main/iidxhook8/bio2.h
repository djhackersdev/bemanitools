#ifndef IIDXHOOK_BIO2_H
#define IIDXHOOK_BIO2_H

#include <windows.h>

#include <stdbool.h>

#include "hook/iohook.h"

void bio2_port_init(bool disable_poll_limiter);
void bio2_port_fini(void);
HRESULT bio2_port_dispatch_irp(struct irp *irp);

#endif

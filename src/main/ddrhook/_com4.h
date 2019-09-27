#ifndef IIDXHOOK_COM4_H
#define IIDXHOOK_COM4_H

#include "hook/iohook.h"

void com4_init(void);
void com4_fini(void);
HRESULT com4_dispatch_irp(struct irp *irp);

#endif

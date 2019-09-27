#ifndef DDRHOOK_USBMEM_H
#define DDRHOOK_USBMEM_H

#include "hook/iohook.h"

void usbmem_init(void);
void usbmem_fini(void);
HRESULT usbmem_dispatch_irp(struct irp *irp);

#endif

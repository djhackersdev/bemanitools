#ifndef DDRHOOK_EXTIO_H
#define DDRHOOK_EXTIO_H

#include <windows.h>

#include "hook/iohook.h"

void extio_init(void);
void extio_fini(void);
HRESULT extio_dispatch_irp(struct irp *irp);

#endif

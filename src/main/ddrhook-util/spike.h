#ifndef DDRHOOK_UTIL_SPIKE_H
#define DDRHOOK_UTIL_SPIKE_H

#include <windows.h>

#include "hook/iohook.h"

void spike_init(void);
void spike_fini(void);
HRESULT spike_dispatch_irp(struct irp *irp);

#endif

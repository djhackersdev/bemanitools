#ifndef UNICORNTAIL_P3IO_H
#define UNICORNTAIL_P3IO_H

#include "hook/iohook.h"

void p3io_filter_init(void);
void p3io_filter_fini(void);
HRESULT p3io_filter_dispatch_irp(struct irp *irp);

#endif

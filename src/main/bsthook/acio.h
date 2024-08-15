#ifndef IIDXHOOK_AC_IO_H
#define IIDXHOOK_AC_IO_H

#include <windows.h>

#include "api/io/bst.h"
#include "api/io/eam.h"

#include "hook/iohook.h"

void ac_io_bus_init(void);
void ac_io_bus_fini(void);
HRESULT ac_io_bus_dispatch_irp(struct irp *irp);

#endif

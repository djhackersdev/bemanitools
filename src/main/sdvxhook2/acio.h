#ifndef SDVXHOOK2_AC_IO_H
#define SDVXHOOK2_AC_IO_H

#include <windows.h>

#include <stdbool.h>

#include "hook/iohook.h"

void ac_io_port_init(bool use_new_reader);
void ac_io_port_fini(void);
HRESULT ac_io_port_dispatch_irp(struct irp *irp);

#endif

#ifndef PNMHOOK2_ACIO_H
#define PNMHOOK2_ACIO_H

#include "hook/iohook.h"

/**
 * Initialize the ACIO backend for jubeat.
 */
void ac_io_port_init(void);

/**
 * Shutdown the ACIO backend.
 */
void ac_io_port_fini(void);

/**
 * ACIO backend dispatch irp function. This needs to be hooked up to the iohook
 * module in order to receive system calls to dispatch for emulation.
 */
HRESULT ac_io_port_dispatch_irp(struct irp *irp);

#endif

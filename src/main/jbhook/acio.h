#ifndef JBHOOK_ACIO_H
#define JBHOOK_ACIO_H

void ac_io_port_init(void);
void ac_io_port_fini(void);
HRESULT ac_io_port_dispatch_irp(struct irp *irp);

#endif

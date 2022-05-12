#ifndef HOOK_USBMEM_H
#define HOOK_USBMEM_H

void usbmem_init(void);
void usbmem_fini(void);
HRESULT usbmem_dispatch_irp(struct irp *irp);

#endif

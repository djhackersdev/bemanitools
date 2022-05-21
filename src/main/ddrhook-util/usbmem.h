#ifndef DDRHOOK_UTIL_USBMEM_H
#define DDRHOOK_UTIL_USBMEM_H

void usbmem_init(const char *path, const bool enabled);
void usbmem_fini(void);
HRESULT usbmem_dispatch_irp(struct irp *irp);

#endif

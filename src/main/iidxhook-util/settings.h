#ifndef IIDXHOOK_SETTINGS_H
#define IIDXHOOK_SETTINGS_H

#include "hook/iohook.h"

/**
 * Remaps the paths for the settings drives d:\, e:\ and f:\
 * to local folders e\ and f\.
 * Needed on IIDX 9th to Sirius.
 */
void settings_hook_init(void);

/**
 * iohook dispatch function
 */
HRESULT settings_hook_dispatch_irp(struct irp *irp);

#endif

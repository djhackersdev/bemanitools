#ifndef IIDXHOOK_UTIL_ACIO_H
#define IIDXHOOK_UTIL_ACIO_H

#include <windows.h>

#include <stdbool.h>

#include "hook/iohook.h"

/**
 * Initialize an emulated ACIO on COM1 for IIDX.
 * 
 * @param legacy_mode Set to true if running on slotted readers on iidx 13-18.
 *        Old games using slotted readers might use an old libacio version that
 *        does not support handling of multi messages per package. Instead,
 *        legacy mode responds with one message per acio package. The game
 *        logs MiCmd Retry errors with the code 0x00000002 if this option is
 *        not set correctly.
 */
void iidxhook_util_acio_init(bool legacy_mode);

/**
 * Cleanup the ACIO emulation layer.
 */
void iidxhook_util_acio_fini(void);

/**
 * iohook dispatch function. Needs to be installed.
 */
HRESULT iidxhook_util_acio_dispatch_irp(struct irp *irp);

#endif

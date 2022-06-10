#ifndef POPNHOOK_UTIL_ACIO_H
#define POPNHOOK_UTIL_ACIO_H

#include <windows.h>

#include <stdbool.h>

#include "acioemu/icca.h"
#include "hook/iohook.h"

/**
 * Initialize an emulated ACIO on COM1 for pop'n music.
 *
 * @param legacy_mode Set to true if running on slotted readers on pop'n 15-18.
 */
void popnhook_acio_init(bool legacy_mode);

/**
 * Use the specified ICCA emulation version
 */
void popnhook_acio_override_version(enum ac_io_emu_icca_version version);

/**
 * Cleanup the ACIO emulation layer.
 */
void popnhook_acio_fini(void);

/**
 * iohook dispatch function. Needs to be installed.
 */
HRESULT popnhook_acio_dispatch_irp(struct irp *irp);

#endif

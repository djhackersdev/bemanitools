#ifndef IIDXHOOK_UTIL_CHART_PATCH_H
#define IIDXHOOK_UTIL_CHART_PATCH_H

#include <windows.h>

#include "hook/iohook.h"

/* 9th to DD */
#define IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_9_TO_13 59.95
/* GOLD to Resort Anthem */
#define IIDXHOOK_UTIL_CHART_PATCH_TIMEBASE_14_TO_18_VGA 60.05

/**
 * Install hooks to intercept chart file loading. This allows you to patch
 * the charts in-memory to modify their timestamps and making the game run
 * on-sync on non-stock setups with different display refresh rates than
 * 59.94 or 60.04 (depending on game version).
 *
 * @param orig_timebase The original refresh rate (timebase value) of the game.
 *        Use one of the defined macros.
 */
void iidxhook_util_chart_patch_init(double orig_timebase);

/**
 * Shut down this module.
 */
void iidxhook_util_chart_patch_fini(void);

/**
 * Set the target refresh rate of the display. Determine this value either
 * by using the d3d8 or d3d9 monitor check of bemanitools or get it from another
 * source (e.g. one of the newer games in-built monitor checks).
 *
 * @param hz Refresh rate of the display. Ensure your display provides a
 *        constant value.
 */
void iidxhook_util_chart_patch_set_refresh_rate(double hz);

/**
 * iohook dispatch function. Needs to be installed.
 */
HRESULT iidxhook_util_chart_patch_dispatch_irp(struct irp *irp);

#endif

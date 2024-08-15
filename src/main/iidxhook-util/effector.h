#ifndef IIDXHOOK_EFFECTOR_H
#define IIDXHOOK_EFFECTOR_H

#include <stdbool.h>

/**
 * Hook rteffects.dll exports (10th to DD)
 */
void effector_hook_init(void);

void effector_hook_fini();

#endif

#ifndef IIDXHOOK_UTIL_CLOCK_H
#define IIDXHOOK_UTIL_CLOCK_H

/**
 * Hook clock related calls to avoid chaning the system clock
 * time from the operator menu. 9th style is a special case here as it
 * allows a max. year of 2009, only.
 */
void iidxhook_util_clock_hook_init(void);

#endif

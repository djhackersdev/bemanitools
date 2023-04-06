#ifndef IIDXHOOK_EZUSB_MON_H
#define IIDXHOOK_EZUSB_MON_H

/**
 * Monitor and log call traces of ezusb.dll calls.
 *
 * This only works on 9th and 10th Style that have the ezusb.dll linked
 * dynamically. Any later version has the library linked staticly
 */
void ezusb_mon_hook_init(void);

#endif

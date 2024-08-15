#ifndef IIDXHOOK_LOG_EZUSB_H
#define IIDXHOOK_LOG_EZUSB_H

/**
 * Hook into the logger call of the built in ezusb logger
 * (this works on 9th to HappySky, only).
 */
void ezusb_log_hook_init(void);

void ezusb_log_hook_fini();

#endif

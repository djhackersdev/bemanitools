#ifndef HOOKLIB_RS232_H
#define HOOKLIB_RS232_H

#include <windows.h>

void rs232_hook_init(void);

/**
 * Allows limiting the rs232 to only the specified fd's
 */

void rs232_hook_limit_hooks(void);

/**
 * Adds the specified fd to the list of descriptors to hook
 *
 * @param fd descriptor to hook
 */
void rs232_hook_add_fd(HANDLE fd);

#endif

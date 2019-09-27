#ifndef CONFIG_USAGES_H
#define CONFIG_USAGES_H

#include <windows.h>

#include <stdint.h>

void usages_init(HINSTANCE inst);
void usages_get(char *chars, uint32_t nchars, uint32_t usage);
void usages_fini(void);

#endif

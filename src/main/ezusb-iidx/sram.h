#ifndef EZUSB_IIDX_SRAM_H
#define EZUSB_IIDX_SRAM_H

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

bool ezusb_iidx_sram_init(HANDLE handle, const void* buffer, uint16_t size);

#endif
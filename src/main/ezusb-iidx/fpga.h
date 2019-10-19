#ifndef EZUSB_IIDX_FPGA_H
#define EZUSB_IIDX_FPGA_H

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

bool ezusb_iidx_fpga_v1_init(HANDLE handle, const void *buffer, uint16_t size);

bool ezusb_iidx_fpga_v2_init(HANDLE handle, const void *buffer, uint16_t size);

#endif
#ifndef EZUSB2_IIDX_H
#define EZUSB2_IIDX_H

#include <stdbool.h>

#include <windows.h>

#include "msg.h"

bool ezusb2_iidx_interrupt_read(
    HANDLE handle, struct ezusb2_iidx_msg_interrupt_read_packet *packet);

bool ezusb2_iidx_interrupt_write(
    HANDLE handle, const struct ezusb2_iidx_msg_interrupt_write_packet *packet);

#endif
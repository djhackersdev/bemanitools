#ifndef GENINPUT_HID_GENERIC_H
#define GENINPUT_HID_GENERIC_H

#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "geninput/hid.h"

bool hid_generic_open(struct hid_fd **hid, const char *dev_node,
        HANDLE iocp, uintptr_t iocp_ctx);

#endif

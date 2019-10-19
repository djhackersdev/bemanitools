#ifndef GENINPUT_PACDRIVE_H
#define GENINPUT_PACDRIVE_H

#include <stdbool.h>
#include <stddef.h>

#include "geninput/io-thread.h"

bool pac_open(
    struct hid_fd **hid_out,
    const char *dev_node,
    HANDLE iocp,
    uintptr_t iocp_ctx);

#endif

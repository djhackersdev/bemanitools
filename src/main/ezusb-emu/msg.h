#ifndef EZUSB_EMU_MSG_H
#define EZUSB_EMU_MSG_H

#include <windows.h>

#include <stddef.h>
#include <stdint.h>

#include "util/iobuf.h"

struct ezusb_emu_msg_hook {
    HRESULT (*interrupt_read)(struct iobuf *read);
    HRESULT (*interrupt_write)(struct const_iobuf *write);
    HRESULT (*bulk_read)(struct iobuf *read);
    HRESULT (*bulk_write)(struct const_iobuf *write);
};

#endif

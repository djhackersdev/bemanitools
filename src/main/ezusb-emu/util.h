#ifndef EZUSB_EMU_UTIL_H
#define EZUSB_EMU_UTIL_H

#include <stdint.h>

#include <windows.h>
#include <setupapi.h>
#include <usb100.h>

#include "ezusb/ezusbsys2.h"

#include "hook/iohook.h"

void ezusb_emu_util_log_usb_msg(const char* prefix, uint32_t ctl_code,
        const BULK_TRANSFER_CONTROL *ctl, uint32_t ctl_size, void* header,
        uint32_t header_bytes, void* data, uint32_t data_bytes);

#endif

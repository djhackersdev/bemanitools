#define LOG_MODULE "ezusb-emu-util"

#include <windows.h>

#include <setupapi.h>
#include <stdio.h>
#include <usb100.h>

#include "ezusb-emu/util.h"

#include "util/hex.h"
#include "util/log.h"

enum ezusb_pipe {
    /* This is just the NT driver API. Add 1 to get the actual EP number. */
    EZUSB_PIPE_INTERRUPT_OUT = 0,
    EZUSB_PIPE_INTERRUPT_IN = 1,
    EZUSB_PIPE_BULK_OUT = 2,
    EZUSB_PIPE_BULK_IN = 3
};

void ezusb_emu_util_log_usb_msg(
    const char *prefix,
    uint32_t ctl_code,
    const BULK_TRANSFER_CONTROL *ctl,
    uint32_t ctl_size,
    void *header,
    uint32_t header_bytes,
    void *data,
    uint32_t data_bytes)
{
    char header_str[4096];
    char data_str[4096];
    const char *ctl_code_str;

    switch (ctl_code) {
        case IOCTL_Ezusb_GET_DEVICE_DESCRIPTOR:
            ctl_code_str = "GET_DEVICE_DESCRIPTOR";
            break;

        case IOCTL_Ezusb_VENDOR_REQUEST:
            ctl_code_str = "VENDOR_REQUEST";
            break;

        case IOCTL_EZUSB_ANCHOR_DOWNLOAD:
            ctl_code_str = "ANCHOR_DOWNLOAD";
            break;

        case IOCTL_EZUSB_BULK_READ:
            if (ctl->pipeNum == EZUSB_PIPE_INTERRUPT_IN) {
                ctl_code_str = "INT_READ";
            } else if (ctl->pipeNum == EZUSB_PIPE_BULK_IN) {
                ctl_code_str = "BULK_READ";
            } else {
                ctl_code_str = "INVALID_READ";
            }

            break;

        case IOCTL_EZUSB_BULK_WRITE:
            if (ctl->pipeNum == EZUSB_PIPE_INTERRUPT_OUT) {
                ctl_code_str = "INT_WRITE";
            } else if (ctl->pipeNum == EZUSB_PIPE_BULK_OUT) {
                ctl_code_str = "BULK_WRITE";
            } else {
                ctl_code_str = "INVALID_WRITE";
            }

            break;

        default:
            ctl_code_str = "UNKNOWN";
            break;
    }

    hex_encode_uc(header, header_bytes, header_str, sizeof(header_str));
    hex_encode_uc(data, data_bytes, data_str, sizeof(data_str));

    log_warning(
        "[EZUSB DUMP %s][%s] header(%d) %s |||| data(%d) %s",
        prefix,
        ctl_code_str,
        header_bytes,
        header_str,
        data_bytes,
        data_str);
}
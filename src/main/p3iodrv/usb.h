#ifndef P3IODRV_USB_H
#define P3IODRV_USB_H

#include <stdbool.h>
#include <stddef.h>
#include <windows.h>

HANDLE p3io_usb_open(void);
void p3io_usb_close(HANDLE p3io_handle);
bool p3io_usb_read_jamma(HANDLE interrupt_handle, uint32_t jamma[3]);

// Not necessarily supported by all drivers but, for example, by a 64-bit driver
bool p3io_usb_read_version(HANDLE bulk_handle, char version[128]);
/*
bool p3io_usb_transfer(
    HANDLE bulk_handle,
    uint8_t cmd,
    uint8_t seq_no,
    const void *req_payload,
    size_t req_payload_len,
    void *resp_payload,
    size_t *resp_payload_len
);
*/

#endif

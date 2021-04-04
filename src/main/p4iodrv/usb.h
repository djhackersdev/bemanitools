#ifndef P4IODRV_USB_H
#define P4IODRV_USB_H

#include <stdbool.h>
#include <stddef.h>
#include <windows.h>

HANDLE p4io_usb_open(void);
void p4io_usb_close(HANDLE p4io_handle);
bool p4io_usb_read_jamma(HANDLE jamma_handle, uint32_t jamma[4]);
bool p4io_usb_read_device_name(HANDLE bulk_handle, char name[128]);
bool p4io_usb_transfer(
    HANDLE bulk_handle,
    uint8_t cmd,
    uint8_t seq_no,
    const void *req_payload,
    size_t req_payload_len,
    void *resp_payload,
    size_t *resp_payload_len
);

#endif

#include <setupapi.h>
#include <windows.h>

#include "ezusb/ezusb.h"
#include "ezusb/ezusbsys2.h"

#include "util/log.h"
#include "util/str.h"

static bool ezusb_reset(HANDLE handle, bool hold)
{
    VENDOR_OR_CLASS_REQUEST_CONTROL req;
    DWORD outpkt;

    /* fml...I can't believe these values are correct. according to
       the documentation they are wrong D: */

    req.direction = 0xA0;

    if (hold) {
        req.requestType = 0x00;
    } else {
        req.requestType = 0x84;
    }

    req.recepient = 0x92;

    req.requestTypeReservedBits = 0x7F;
    req.request = 0x00;
    req.value = 0x0001;

    if (hold) {
        req.index = 0x0100;
    } else {
        /* release */
        req.index = 0x0000;
    }

    if (!DeviceIoControl(
            handle,
            IOCTL_Ezusb_VENDOR_REQUEST,
            &req,
            sizeof(req),
            &req,
            sizeof(req),
            &outpkt,
            NULL)) {
        return false;
    }

    return true;
}

static bool ezusb_write_firmware(
    HANDLE handle, const void *buffer, uint16_t ram_offset, uint32_t size)
{
    ANCHOR_DOWNLOAD_CONTROL req;
    DWORD outpkt;

    req.Offset = ram_offset;

    if (!DeviceIoControl(
            handle,
            IOCTL_EZUSB_ANCHOR_DOWNLOAD,
            &req,
            sizeof(req),
            (void *) buffer,
            size,
            &outpkt,
            NULL)) {
        return false;
    }

    return true;
}

HANDLE
ezusb_open(const char *path)
{
    log_assert(path);

    return CreateFileA(
        path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
}

bool ezusb_get_ident(HANDLE handle, struct ezusb_ident *ident)
{
    USB_DEVICE_DESCRIPTOR desc;
    DWORD outpkt;

    log_assert(handle);
    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(ident);

    if (!DeviceIoControl(
            handle,
            IOCTL_Ezusb_GET_DEVICE_DESCRIPTOR,
            &desc,
            sizeof(desc),
            &desc,
            sizeof(desc),
            &outpkt,
            NULL)) {
        return false;
    }

    memset(ident->name, 0, sizeof(ident->name));
    ident->vid = desc.idVendor;
    ident->pid = desc.idProduct;

    return true;
}

bool ezusb_download_firmware(HANDLE handle, struct ezusb_firmware *fw)
{
    log_assert(handle);
    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(fw);

    if (!ezusb_reset(handle, true)) {
        return false;
    }

    for (uint16_t i = 0; i < fw->segment_count; i++) {
        if (!ezusb_write_firmware(
                handle,
                fw->segments[i]->data,
                fw->segments[i]->offset,
                fw->segments[i]->size)) {
            return false;
        }
    }

    if (!ezusb_reset(handle, false)) {
        return false;
    }

    return true;
}

void ezusb_close(HANDLE handle)
{
    log_assert(handle);
    log_assert(handle != INVALID_HANDLE_VALUE);

    CloseHandle(handle);
}
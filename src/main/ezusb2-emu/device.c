#define LOG_MODULE "ezusb2-emu-device"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
#include <usb100.h>
// clang-format on

#include <string.h>

#include "ezusb/util.h"

#include "ezusb2/cyioctl.h"

#include "ezusb-emu/msg.h"

#include "ezusb2-emu/conf.h"
#include "ezusb2-emu/desc.h"
#include "ezusb2-emu/device.h"
#include "ezusb2-emu/util.h"

#include "hook/iohook.h"

#include "imports/avs.h"

#include "util/fs.h"
#include "util/hex.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

enum pipe {
    PIPE_INT_OUT = 0x01,
    PIPE_BULK_OUT = 0x02,
    PIPE_INT_IN = 0x81,
    /* yes, 0x86 and NOT 0x82... */
    PIPE_BULK_IN = 0x86
};

struct ezusb_usb_string_desc {
    uint8_t length;
    uint8_t desc_type;
    /* Max length? 9 elements set by game... */
    uint16_t unicode_str[9];
};

static HRESULT ezusb_open(struct irp *irp);
static HRESULT ezusb_ioctl(struct irp *irp);

static HRESULT ezusb_ioctl_ep0(
    SINGLE_TRANSFER *usb_req, struct const_iobuf *write, struct iobuf *read);

static HRESULT ezusb_ioctl_epX(
    SINGLE_TRANSFER *usb_req, struct const_iobuf *write, struct iobuf *read);

static HRESULT ezusb_get_device_descriptor(struct iobuf *read);
static HRESULT ezusb_get_string_descriptor(struct iobuf *read);
static HRESULT ezusb_reset(struct const_iobuf *write);
static HRESULT ezusb_upload_fw(uint16_t offset, struct const_iobuf *write);

static HANDLE ezusb_fd;
static struct ezusb_firmware *ezusb_emu_firmware;
static struct ezusb_emu_msg_hook *ezusb_emu_dev_fx2_msg_hook;

void ezusb2_emu_device_hook_init(struct ezusb_emu_msg_hook *msg_hook)
{
    log_assert(ezusb_fd == NULL);

    ezusb_fd = iohook_open_dummy_fd();
    ezusb_emu_dev_fx2_msg_hook = msg_hook;
}

void ezusb2_emu_device_hook_fini(void)
{
    if (ezusb_fd != NULL) {
        CloseHandle(ezusb_fd);
    }

    ezusb_fd = NULL;
}

/*
 * WIN32 I/O AND IOHOOK LAYER
 */

HRESULT
ezusb2_emu_device_dispatch_irp(struct irp *irp)
{
    if (irp->op != IRP_OP_OPEN && irp->fd != ezusb_fd) {
        return irp_invoke_next(irp);
    }

    /* read/write are not supported, and the game-side EZUSB code constantly
       churns through FDs opening and closing them (so we silently acknowledge
       CloseHandle calls and don't even log them). */

    switch (irp->op) {
        case IRP_OP_OPEN:
            return ezusb_open(irp);
        case IRP_OP_CLOSE:
            return S_OK;
        case IRP_OP_IOCTL:
            return ezusb_ioctl(irp);
        default:
            return E_NOTIMPL;
    }
}

static HRESULT ezusb_open(struct irp *irp)
{
    log_assert(irp != NULL);

    if (!wstr_eq(irp->open_filename, L"\\\\.\\Ezusb-0")) {
        return irp_invoke_next(irp);
    }

    irp->fd = ezusb_fd;

    return S_OK;
}

static HRESULT ezusb_ioctl(struct irp *irp)
{
    SINGLE_TRANSFER *usb_req;
    struct const_iobuf write;
    struct iobuf read;
    HRESULT hr;

    log_assert(irp != NULL);

#ifdef EZUSB_EMU_DEBUG_DUMP
    /* For debugging */
    ezusb2_emu_util_log_usb_msg("BEFORE", irp);
#endif

    if (irp->write.nbytes < sizeof(SINGLE_TRANSFER)) {
        log_warning("SINGLE_TRANSFER out buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    if (irp->read.nbytes < sizeof(SINGLE_TRANSFER)) {
        log_warning("SINGLE_TRANSFER in buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    memcpy(irp->read.bytes, irp->write.bytes, sizeof(SINGLE_TRANSFER));

    write.bytes = irp->write.bytes + sizeof(SINGLE_TRANSFER);
    write.nbytes = irp->write.nbytes - sizeof(SINGLE_TRANSFER);
    write.pos = 0;
    read.bytes = irp->read.bytes + sizeof(SINGLE_TRANSFER);
    read.nbytes = irp->read.nbytes - sizeof(SINGLE_TRANSFER);
    read.pos = 0;

    usb_req = (SINGLE_TRANSFER *) irp->write.bytes;

    switch (irp->ioctl) {
        case IOCTL_ADAPT_SEND_EP0_CONTROL_TRANSFER:
            hr = ezusb_ioctl_ep0(usb_req, &write, &read);

            break;

        case IOCTL_ADAPT_SEND_NON_EP0_TRANSFER:
            hr = ezusb_ioctl_epX(usb_req, &write, &read);

            break;

        default:
            log_warning("Unknown ioctl %08x", irp->ioctl);
            hr = E_INVALIDARG;

            break;
    }

    if (FAILED(hr)) {
        return hr;
    }

    irp->read.pos = read.pos + sizeof(SINGLE_TRANSFER);

#ifdef EZUSB_EMU_DEBUG_DUMP
    /* For debugging */
    ezusb2_emu_util_log_usb_msg("AFTER", irp);
#endif

    return hr;
}

/*
 * USB TRANSFER LAYER
 */

static HRESULT ezusb_ioctl_ep0(
    SINGLE_TRANSFER *usb_req, struct const_iobuf *write, struct iobuf *read)
{
    log_assert(usb_req != NULL);
    log_assert(write != NULL);
    log_assert(read != NULL);

    /*
       0x80: Device -> Host Request, Standard, Recipient Device
       0x40: Host -> Device Request, Class, Recipient Device
    */

    if (usb_req->SetupPacket.bmRequest == 0x80 &&
        usb_req->SetupPacket.bRequest == 0x06) {
        switch (usb_req->SetupPacket.wValue) {
            /* Get usb device descriptor */
            case 0x0100:
                /* return data with USB_DEVICE_DESCRIPTOR struct */
                return ezusb_get_device_descriptor(read);

            /* get usb string descriptor */
            case 0x0301:
                return ezusb_get_string_descriptor(read);

            default:
                log_warning(
                    "Unsupported standard req (dev->host) value: %04X",
                    usb_req->SetupPacket.wValue);

                return E_INVALIDARG;
        }
    } else if (
        usb_req->SetupPacket.bmRequest == 0x40 &&
        usb_req->SetupPacket.bRequest == 0xA0) {
        if (usb_req->SetupPacket.wValue == 0xE600) {
            /* reset */
            return ezusb_reset(write);
        } else {
            /* fw download */
            return ezusb_upload_fw(usb_req->SetupPacket.wValue, write);
        }
    } else {
        log_warning(
            "Invalid standard req_type and/or request: %02X %02X",
            usb_req->SetupPacket.bmRequest,
            usb_req->SetupPacket.bRequest);

        return E_INVALIDARG;
    }
}

static HRESULT ezusb_ioctl_epX(
    SINGLE_TRANSFER *usb_req, struct const_iobuf *write, struct iobuf *read)
{
    log_assert(usb_req != NULL);
    log_assert(write != NULL);
    log_assert(read != NULL);

    /* No reason to follow a standard, right? */
    switch (usb_req->ucEndpointAddress) {
        case PIPE_INT_OUT:
            return ezusb_emu_dev_fx2_msg_hook->interrupt_write(write);

        case PIPE_BULK_OUT:
            return ezusb_emu_dev_fx2_msg_hook->bulk_write(write);

        case PIPE_INT_IN:
            return ezusb_emu_dev_fx2_msg_hook->interrupt_read(read);

        case PIPE_BULK_IN:
            return ezusb_emu_dev_fx2_msg_hook->bulk_read(read);

        default:
            log_warning("Unhandled endpoint %02X", usb_req->ucEndpointAddress);

            return E_INVALIDARG;
    }
}

static HRESULT ezusb_get_device_descriptor(struct iobuf *read)
{
    USB_DEVICE_DESCRIPTOR *desc;

    log_assert(read != NULL);

    if (read->nbytes < sizeof(*desc)) {
        log_warning("USB_DEVICE_DESCRIPTOR buffer too small: %d", read->nbytes);

        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }

    desc = (USB_DEVICE_DESCRIPTOR *) read->bytes;

    /* vid and pid checked, only */
    desc->idVendor = ezusb2_emu_desc_device.vid;
    desc->idProduct = ezusb2_emu_desc_device.pid;

    log_misc(
        "get_device_descriptor: vid %02x, pid %02x",
        desc->idVendor,
        desc->idProduct);

    read->pos = sizeof(*desc);

    return S_OK;
}

static HRESULT ezusb_get_string_descriptor(struct iobuf *read)
{
    struct ezusb_usb_string_desc *desc;

    log_assert(read != NULL);

    if (read->nbytes < sizeof(*desc)) {
        log_warning("ezusb_usb_string_desc buffer too small: %d", read->nbytes);

        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }

    desc = (struct ezusb_usb_string_desc *) read->bytes;

    desc->length = sizeof(*desc);
    desc->desc_type = 0x03; /* Usb spec says so */
    memcpy(desc->unicode_str, L"KONAMI", 12); /* Unicode encoding */

    read->pos = sizeof(*desc);

    log_misc("get_string_descriptor: KONAMI");

    return S_OK;
}

static HRESULT ezusb_reset(struct const_iobuf *write)
{
    log_assert(write != NULL);

    switch (write->bytes[0]) {
        case 0x01:
            log_misc("reset hold, starting fw download...");
            ezusb_emu_firmware = ezusb_firmware_alloc();

            return S_OK;

        case 0x00:
            log_misc("reset release, finished fw download");

            ezusb_emu_firmware->crc = ezusb_firmware_crc(ezusb_emu_firmware);

#ifdef EZUSB_EMU_FW_DUMP
            if (!ezusb_firmware_save("ezusb_fx2.bin", ezusb_emu_firmware)) {
                log_fatal("Saving dumped firmware failed");
            } else {
                log_misc("firmware dumped do ezusb_fx2.bin file");
            }

            free(ezusb_emu_firmware);
            ezusb_emu_firmware = NULL;
#endif

            return S_OK;

        default:
            log_warning("Unknown reset cmd: %02X", write->bytes[0]);

            return E_FAIL;
    }
}

static HRESULT ezusb_upload_fw(uint16_t offset, struct const_iobuf *write)
{
    log_misc("upload_fw, offset %04X, nbytes %04X", offset, write->nbytes);

    ezusb_firmware_add_segment(
        ezusb_emu_firmware,
        ezusb_firmware_segment_alloc(
            offset, write->nbytes, (void *) write->bytes));

    return S_OK;
}

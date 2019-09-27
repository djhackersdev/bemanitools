#define LOG_MODULE "ezusb-emu-device"

#include <windows.h>
#include <setupapi.h>
#include <usb100.h>

#include <string.h>

#include "ezusb/ezusbsys2.h"
#include "ezusb/util.h"

#include "ezusb-emu/conf.h"
#include "ezusb-emu/desc.h"
#include "ezusb-emu/device.h"
#include "ezusb-emu/msg.h"
#include "ezusb-emu/util.h"

#include "hook/iohook.h"

#include "imports/avs.h"

#include "util/fs.h"
#include "util/hex.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static HRESULT ezusb_get_device_descriptor(struct irp *irp);
static HRESULT ezusb_vendor_req(struct irp *irp);
static HRESULT ezusb_upload_fw(struct irp *irp);
static HRESULT ezusb_pipe_read(struct irp *irp);
static HRESULT ezusb_pipe_write(struct irp *irp);

static HRESULT ezusb_open(struct irp *irp);
static HRESULT ezusb_ioctl(struct irp *irp);

enum ezusb_pipe {
    /* This is just the NT driver API. Add 1 to get the actual EP number. */
    EZUSB_PIPE_INTERRUPT_OUT = 0,
    EZUSB_PIPE_INTERRUPT_IN = 1,
    EZUSB_PIPE_BULK_OUT = 2,
    EZUSB_PIPE_BULK_IN = 3
};

static HANDLE ezusb_emu_fd;
static struct ezusb_firmware* ezusb_emu_firmware;
static struct ezusb_emu_msg_hook* ezusb_emu_dev_fx_msg_hook;

void ezusb_emu_device_hook_init(struct ezusb_emu_msg_hook* msg_hook)
{
    log_assert(ezusb_emu_fd == NULL);

    ezusb_emu_fd = iohook_open_dummy_fd();
    ezusb_emu_dev_fx_msg_hook = msg_hook;
}

void ezusb_emu_device_hook_fini(void)
{
    if (ezusb_emu_fd != NULL) {
        CloseHandle(ezusb_emu_fd);
    }

    ezusb_emu_fd = NULL;
}

HRESULT ezusb_emu_device_dispatch_irp(struct irp *irp)
{
    if (irp->op != IRP_OP_OPEN && irp->fd != ezusb_emu_fd) {
        return irp_invoke_next(irp);
    }

    /* read/write are not supported, and the game-side EZUSB code constantly
       churns through FDs opening and closing them (so we silently acknowledge
       CloseHandle calls and don't even log them). */

    switch (irp->op) {
    case IRP_OP_OPEN:   return ezusb_open(irp);
    case IRP_OP_CLOSE:  return S_OK;
    case IRP_OP_IOCTL:  return ezusb_ioctl(irp);
    default:            return E_NOTIMPL;
    }
}

/*
 * WIN32 I/O AND IOHOOK LAYER
 */

static HRESULT ezusb_open(struct irp *irp)
{
    log_assert(irp != NULL);

    if (!wstr_eq(irp->open_filename, L"\\\\.\\Ezusb-0")) {
        return irp_invoke_next(irp);
    }

    irp->fd = ezusb_emu_fd;

    return S_OK;
}

static HRESULT ezusb_ioctl(struct irp *irp)
{
    /* For debugging */
#ifdef EZUSB_EMU_DEBUG_DUMP
    /* For debugging */
    ezusb_emu_util_log_usb_msg("BEFORE", irp->ioctl, irp->read.bytes, 
        irp->read.nbytes, irp->read.bytes, irp->read.nbytes, irp->write.bytes, 
        irp->write.nbytes);
#endif

    /* Cases are listed in order of first receipt */
    switch (irp->ioctl) {
        case IOCTL_Ezusb_GET_DEVICE_DESCRIPTOR:
            return ezusb_get_device_descriptor(irp);

        case IOCTL_Ezusb_VENDOR_REQUEST:
            return ezusb_vendor_req(irp);

        case IOCTL_EZUSB_ANCHOR_DOWNLOAD:
            return ezusb_upload_fw(irp);

        case IOCTL_EZUSB_BULK_READ:
            /* Misnomer: can be bulk or interrupt. */
            return ezusb_pipe_read(irp);

        case IOCTL_EZUSB_BULK_WRITE:
            /* Ditto. */
            return ezusb_pipe_write(irp);

        default:
            log_warning("Unknown ioctl %08x", irp->ioctl);

            return E_INVALIDARG;
    }

#ifdef EZUSB_EMU_DEBUG_DUMP
    /* For debugging */
    ezusb_emu_util_log_usb_msg("AFTER", irp->ioctl, irp->read.bytes, 
        irp->read.nbytes, irp->read.bytes, irp->read.nbytes, irp->write.bytes, 
        irp->write.nbytes);
#endif
}
/*
 * USB TRANSFER LAYER
 */

static HRESULT ezusb_get_device_descriptor(struct irp *irp)
{
    USB_DEVICE_DESCRIPTOR *desc;

    log_assert(irp != NULL);

    if (irp->read.nbytes < sizeof(*desc)) {
        log_warning("USB_DEVICE_DESCRIPTOR buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    desc = (USB_DEVICE_DESCRIPTOR *) irp->read.bytes;

    memset(desc, 0, sizeof(*desc));
    desc->idVendor = ezusb_emu_desc_device.vid;
    desc->idProduct = ezusb_emu_desc_device.pid;
    irp->read.pos = sizeof(*desc);

    log_misc(
            "get_device_descriptor: vid %02x, pid %02x",
            desc->idVendor,
            desc->idProduct);

    return S_OK;
}

static HRESULT ezusb_vendor_req(struct irp *irp)
{
    VENDOR_OR_CLASS_REQUEST_CONTROL *vc;

    log_assert(irp != NULL);

    if (irp->write.nbytes < sizeof(*vc)) {
        log_warning("VENDOR_OR_CLASS_REQUEST_CONTROL buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    vc = (VENDOR_OR_CLASS_REQUEST_CONTROL *) irp->write.bytes;

    log_misc(
            "vendor req %02x, value %04x, index %04x",
            vc->request,
            vc->value,
            vc->index);

    if (vc->request == 0x00 && vc->value == 0x0001 && vc->index == 0x0100) {
        log_misc("vendor req: reset hold, starting fw download...");
        ezusb_emu_firmware = ezusb_firmware_alloc();
    } else if (vc->request == 0x00 && vc->value == 0x0001 && vc->index == 0x0000) {
        log_misc("vendor req: reset release, finished fw download");
        /* FW download finished, reset 8051 and start the downloaded FW */

        ezusb_emu_firmware->crc = ezusb_firmware_crc(ezusb_emu_firmware);

#ifdef EZUSB_EMU_FW_DUMP
        if (!ezusb_firmware_save("ezusb_fx.bin", ezusb_emu_firmware)) {
            log_fatal("Saving dumped firmware failed");
        } else {
            log_misc("firmware dumped do ezusb_fx.bin file");
        }
#endif

        free(ezusb_emu_firmware);
        ezusb_emu_firmware = NULL;
    } else {
        log_warning("VENDOR_OR_CLASS_REQUEST_CONTROL unknown request");
    }

    return S_OK;
}

static HRESULT ezusb_upload_fw(struct irp *irp)
{
    ANCHOR_DOWNLOAD_CONTROL *hdr;

    log_assert(irp != NULL);

    if (irp->write.nbytes < sizeof(*hdr)) {
        log_warning("ANCHOR_DOWNLOAD_CONTROL buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    hdr = (ANCHOR_DOWNLOAD_CONTROL *) irp->write.bytes;

    /* The semantics for this ioctl are FUCKED! The second buffer is
       supposed to receive data, not send it! */

    log_misc(
            "upload_fw: offset: %04x, nbytes: %04x",
            hdr->Offset,
            irp->read.nbytes);
    ezusb_firmware_add_segment(ezusb_emu_firmware, 
        ezusb_firmware_segment_alloc(hdr->Offset, irp->read.nbytes, 
        (void*) irp->read.bytes));

    return S_OK;
}

static HRESULT ezusb_pipe_read(struct irp *irp)
{
    BULK_TRANSFER_CONTROL *ctl;

    log_assert(irp != NULL);

    if (irp->write.nbytes < sizeof(*ctl)) {
        log_warning("BULK_TRANSFER_CONTROL buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    ctl = (BULK_TRANSFER_CONTROL *) irp->write.bytes;

    switch (ctl->pipeNum) {
    case EZUSB_PIPE_INTERRUPT_IN:
        return ezusb_emu_dev_fx_msg_hook->interrupt_read(&irp->read);

    case EZUSB_PIPE_BULK_IN:
        return ezusb_emu_dev_fx_msg_hook->bulk_read(&irp->read);

    default:
        log_warning("No such read pipe: %u", (unsigned int) ctl->pipeNum);

        return E_INVALIDARG;
    }
}

static HRESULT ezusb_pipe_write(struct irp *irp)
{
    BULK_TRANSFER_CONTROL *ctl;
    struct const_iobuf write;

    if (irp->write.nbytes < sizeof(*ctl)) {
        log_warning("BULK_TRANSFER_CONTROL buffer too small");

        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    ctl = (BULK_TRANSFER_CONTROL *) irp->write.bytes;

    /* ugh */
    write.bytes = irp->read.bytes;
    write.nbytes = irp->read.nbytes;
    write.pos = 0;

    switch (ctl->pipeNum) {
    case EZUSB_PIPE_INTERRUPT_OUT:
        return ezusb_emu_dev_fx_msg_hook->interrupt_write(&write);

    case EZUSB_PIPE_BULK_OUT:
        return ezusb_emu_dev_fx_msg_hook->bulk_write(&write);

    default:
        log_warning("No such write pipe: %u", (unsigned int) ctl->pipeNum);

        return E_INVALIDARG;
    }
}



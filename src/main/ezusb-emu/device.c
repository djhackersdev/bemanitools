#define LOG_MODULE "ezusb-emu-device"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
#include <usb100.h>
// clang-format on

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

// The max buffer size in iidx's ezusb client library is 4096 for the initial
// firmware download.
#define MAX_IOCTL_BUFFER_SIZE 4096

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
static struct ezusb_firmware *ezusb_emu_firmware;
static struct ezusb_emu_msg_hook *ezusb_emu_dev_fx_msg_hook;

void ezusb_emu_device_hook_init(struct ezusb_emu_msg_hook *msg_hook)
{
    log_assert(ezusb_emu_fd == NULL);

    HRESULT hr;

    hr = iohook_open_nul_fd(&ezusb_emu_fd);

    if (hr != S_OK) {
        log_fatal("Opening nul fd failed: %08lx", hr);
    }

    ezusb_emu_dev_fx_msg_hook = msg_hook;
}

void ezusb_emu_device_hook_fini(void)
{
    if (ezusb_emu_fd != NULL) {
        CloseHandle(ezusb_emu_fd);
    }

    ezusb_emu_fd = NULL;
}

HRESULT
ezusb_emu_device_dispatch_irp(struct irp *irp)
{
    if (irp->op != IRP_OP_OPEN && irp->fd != ezusb_emu_fd) {
        return iohook_invoke_next(irp);
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

/*
 * WIN32 I/O AND IOHOOK LAYER
 */

static HRESULT ezusb_open(struct irp *irp)
{
    log_assert(irp != NULL);

    if (!wstr_eq(irp->open_filename, L"\\\\.\\Ezusb-0")) {
        return iohook_invoke_next(irp);
    }

    irp->fd = ezusb_emu_fd;

    return S_OK;
}

static HRESULT ezusb_ioctl(struct irp *irp)
{
    HRESULT res;

    // Stack alloc'd and fixed sized buffers to avoid processing costs with
    // allocations. Ensure buffers are large enough for any operation
    uint8_t write_buffer_local[MAX_IOCTL_BUFFER_SIZE];
    uint8_t read_buffer_local[MAX_IOCTL_BUFFER_SIZE];

    uint8_t *write_buffer_orig;
    uint8_t *read_buffer_orig;

    // Save original buffer that is owned and managed by the caller/the game
    // Do NOT read/write these buffers directly because the game's ezusb
    // interface library does not access to them entirely thread safe.
    // This causes verious odd bugs due to data read/write inconsistencies
    // between data access and modification by at least two different threads.
    //
    // The game's ezusb (client) library was created on a platform with no
    // true multi-core processing (Pentium 4 era of hardware). However, the
    // developers utilized threading primitives of the Win32 API to ensure
    // a high rate of data updates to reduce input latency. This was architected
    // using a dedicated polling thread in the ezusb client library that drives
    // the IO polling on a high update rate. The captured data was stored in
    // a shared buffer that is also accessible by other threads from the main
    // game binary. However, the data access to the same shared buffer with the
    // polling backend is not synchronized
    //
    // Thus, the various odd and flaky ezusb communication bugs we see on
    // modern, and true multi-core hardware, are the concurrency management
    // mistakes that are now creeping up. The developers back then had no
    // proper means to test these due to the lack of hardware capabilities.

    // Save the IO buffer that is used by the ezusb client backend and shared
    // with the game's main thread
    write_buffer_orig = (uint8_t *) irp->write.bytes;
    read_buffer_orig = irp->read.bytes;

    // Prepare our own thread locally managed and non shared buffers for any
    // further data operations that are part of the ezusb emulation stack
    memset(write_buffer_local, 0, sizeof(write_buffer_local));
    memset(read_buffer_local, 0, sizeof(read_buffer_local));

    // Sanity check and visibility, in case this ever overflows
    if (irp->write.nbytes > sizeof(write_buffer_local)) {
        log_fatal(
            "Insufficient local write buffer available for ioctl, local "
            "size %d, ioctl buffer size %d",
            sizeof(write_buffer_local),
            irp->write.nbytes);
    }

    if (irp->read.nbytes > sizeof(read_buffer_local)) {
        log_fatal(
            "Insufficient local read buffer available for ioctl, local "
            "size %d, ioctl buffer size %d",
            sizeof(read_buffer_local),
            irp->read.nbytes);
    }

    // Temporarily hook our local buffers to the irp
    irp->write.bytes = write_buffer_local;
    irp->read.bytes = read_buffer_local;

    // Move data from the shared buffers to the local one
    // Probably the "most atomic way possible" to have the least amount of
    // overlap with the game's shared buffer
    memcpy(write_buffer_local, write_buffer_orig, irp->write.nbytes);
    memcpy(read_buffer_local, read_buffer_orig, irp->read.nbytes);

    /* For debugging */
#ifdef EZUSB_EMU_DEBUG_DUMP
    /* For debugging */
    ezusb_emu_util_log_usb_msg(
        "BEFORE",
        irp->ioctl,
        read_buffered.bytes,
        read_buffered.nbytes,
        read_buffered.bytes,
        read_buffered.nbytes,
        write_buffered.bytes,
        write_buffered.nbytes);
#endif

    /* Cases are listed in order of first receipt */
    switch (irp->ioctl) {
        case IOCTL_Ezusb_GET_DEVICE_DESCRIPTOR:
            res = ezusb_get_device_descriptor(irp);
            break;

        case IOCTL_Ezusb_VENDOR_REQUEST:
            res = ezusb_vendor_req(irp);
            break;

        case IOCTL_EZUSB_ANCHOR_DOWNLOAD:
            res = ezusb_upload_fw(irp);
            break;

        case IOCTL_EZUSB_BULK_READ:
            /* Misnomer: can be bulk or interrupt. */
            res = ezusb_pipe_read(irp);
            break;

        case IOCTL_EZUSB_BULK_WRITE:
            /* Ditto. */
            res = ezusb_pipe_write(irp);
            break;

        default:
            log_warning("Unknown ioctl %08x", irp->ioctl);

            res = E_INVALIDARG;
    }

#ifdef EZUSB_EMU_DEBUG_DUMP
    /* For debugging */
    ezusb_emu_util_log_usb_msg(
        "AFTER",
        irp->ioctl,
        read_buffered.bytes,
        read_buffered.nbytes,
        read_buffered.bytes,
        read_buffered.nbytes,
        write_buffered.bytes,
        write_buffered.nbytes);
#endif

    // Move data back to shared IO buffer. Again, keep this keeps the access
    // overlap as minimal as possible
    memcpy(write_buffer_orig, write_buffer_local, irp->write.nbytes);
    memcpy(read_buffer_orig, read_buffer_local, irp->read.nbytes);

    // Re-store the original irp buffer state
    irp->write.bytes = (const uint8_t *) write_buffer_orig;
    irp->read.bytes = read_buffer_orig;

    return res;
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
    } else if (
        vc->request == 0x00 && vc->value == 0x0001 && vc->index == 0x0000) {
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
        "upload_fw: offset: %04x, nbytes: %04x", hdr->Offset, irp->read.nbytes);
    ezusb_firmware_add_segment(
        ezusb_emu_firmware,
        ezusb_firmware_segment_alloc(
            hdr->Offset, irp->read.nbytes, (void *) irp->read.bytes));

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

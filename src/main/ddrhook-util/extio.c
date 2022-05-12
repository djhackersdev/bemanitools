// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <devioctl.h>
#include <ntdef.h>
#include <ntddser.h>
// clang-format on

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "bemanitools/ddrio.h"

#include "hook/iohook.h"

#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

static HRESULT extio_open(struct irp *irp);
static HRESULT extio_close(struct irp *irp);
static HRESULT extio_write(struct irp *irp);
static HRESULT extio_read(struct irp *irp);
static HRESULT extio_ioctl(struct irp *irp);

static HANDLE extio_fd;
static bool extio_pending;

void extio_init(void)
{
    log_assert(extio_fd == NULL);

    HRESULT hr;

    hr = iohook_open_nul_fd(&extio_fd);

    if (hr != S_OK) {
        log_fatal("Opening nul fd failed: %08lx", hr);
    }
}

void extio_fini(void)
{
    if (extio_fd != NULL) {
        CloseHandle(extio_fd);
    }

    extio_fd = NULL;
}

HRESULT
extio_dispatch_irp(struct irp *irp)
{
    log_assert(irp != NULL);

    if (irp->op != IRP_OP_OPEN && irp->fd != extio_fd) {
        return iohook_invoke_next(irp);
    }

    switch (irp->op) {
        case IRP_OP_OPEN:
            return extio_open(irp);
        case IRP_OP_CLOSE:
            return extio_close(irp);
        case IRP_OP_READ:
            return extio_read(irp);
        case IRP_OP_WRITE:
            return extio_write(irp);
        case IRP_OP_IOCTL:
            return extio_ioctl(irp);
        default:
            return E_NOTIMPL;
    }
}

static HRESULT extio_open(struct irp *irp)
{
    log_assert(irp != NULL);

    if (!wstr_eq(irp->open_filename, L"COM1")) {
        return iohook_invoke_next(irp);
    }

    log_info("EXTIO RS232 port opened");
    irp->fd = extio_fd;

    return S_OK;
}

static HRESULT extio_close(struct irp *irp)
{
    log_info("EXTIO RS232 port closed");

    return S_OK;
}

static HRESULT extio_write(struct irp *irp)
{
    const uint32_t *lights_be;
    uint32_t lights;

    log_assert(irp != NULL);
    log_assert(irp->write.bytes != NULL);

    if (irp->write.nbytes >= sizeof(lights)) {
        lights_be = (const uint32_t *) irp->write.bytes;
        lights = _byteswap_ulong(*lights_be);
        ddr_io_set_lights_extio(lights);

        extio_pending = true;
    } else {
        log_warning("Short EXTIO write");
    }

    irp->write.pos = irp->write.nbytes;

    return S_OK;
}

static HRESULT extio_read(struct irp *irp)
{
    log_assert(irp != NULL);
    log_assert(irp->read.bytes != NULL);

    if (extio_pending && irp->read.nbytes > 0) {
        irp->read.bytes[0] = 0x11;
        irp->read.pos = 1;
        extio_pending = false;
    }

    return S_OK;
}

static HRESULT extio_ioctl(struct irp *irp)
{
    SERIAL_STATUS *status;

    log_assert(irp != NULL);

    switch (irp->ioctl) {
        case IOCTL_SERIAL_GET_COMMSTATUS:
            if (irp->read.bytes == NULL) {
                log_warning(
                    "IOCTL_SERIAL_GET_COMMSTATUS: Output buffer is NULL");

                return E_INVALIDARG;
            }

            if (irp->read.nbytes < sizeof(*status)) {
                log_warning("IOCTL_SERIAL_GET_COMMSTATUS: Buffer is too small");

                return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }

            status = (SERIAL_STATUS *) irp->read.bytes;
            status->Errors = 0;
            status->AmountInInQueue = extio_pending ? 1 : 0;

            irp->read.pos = sizeof(*status);

            break;

        default:
            break;
    }

    return S_OK;
}

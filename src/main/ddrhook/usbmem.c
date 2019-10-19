#define LOG_MODULE "usbmem"

#include <windows.h>

#include <devioctl.h>
#include <ntddser.h>
#include <ntdef.h>

#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "hook/iohook.h"

#include "util/iobuf.h"
#include "util/log.h"
#include "util/str.h"

#define USBMEM_BUF_SIZE 128

static HANDLE usbmem_fd;
static char usbmem_response[USBMEM_BUF_SIZE];
static bool usbmem_pending;

static HRESULT usbmem_open(struct irp *irp);
static HRESULT usbmem_close(struct irp *irp);
static HRESULT usbmem_write(struct irp *irp);
static HRESULT usbmem_read(struct irp *irp);
static HRESULT usbmem_ioctl(struct irp *irp);

void usbmem_init(void)
{
    log_assert(usbmem_fd == NULL);

    usbmem_fd = iohook_open_dummy_fd();
}

void usbmem_fini(void)
{
    if (usbmem_fd != NULL) {
        CloseHandle(usbmem_fd);
    }

    usbmem_fd = NULL;
}

HRESULT
usbmem_dispatch_irp(struct irp *irp)
{
    log_assert(irp != NULL);

    if (irp->op != IRP_OP_OPEN && irp->fd != usbmem_fd) {
        return irp_invoke_next(irp);
    }

    switch (irp->op) {
        case IRP_OP_OPEN:
            return usbmem_open(irp);
        case IRP_OP_CLOSE:
            return usbmem_close(irp);
        case IRP_OP_READ:
            return usbmem_read(irp);
        case IRP_OP_WRITE:
            return usbmem_write(irp);
        case IRP_OP_IOCTL:
            return usbmem_ioctl(irp);
        default:
            return E_NOTIMPL;
    }
}

static HRESULT usbmem_open(struct irp *irp)
{
    log_assert(irp != NULL);

    if (!wstr_eq(irp->open_filename, L"COM3")) {
        return irp_invoke_next(irp);
    }

    irp->fd = usbmem_fd;
    log_info("USB edit data PCB opened");

    return S_OK;
}

static HRESULT usbmem_close(struct irp *irp)
{
    log_info("USB edit data PCB closed");

    return S_OK;
}

static HRESULT usbmem_write(struct irp *irp)
{
    struct const_iobuf *src;
    char request[USBMEM_BUF_SIZE];
    uint32_t nbytes;

    log_assert(irp != NULL);
    log_assert(irp->write.bytes != NULL);

    src = &irp->write;
    nbytes = src->nbytes > USBMEM_BUF_SIZE ? USBMEM_BUF_SIZE : src->nbytes;
    memcpy(request, src->bytes, nbytes);
    request[nbytes - 1] = '\0'; /* This is always a CR. */

    log_misc(">%s", request);

    if (strlen(request) > 0) {
        if (str_eq(request, "sver")) {
            str_cpy(
                usbmem_response,
                sizeof(usbmem_response),
                "done GQHDXJAA DJHACKRS");
        } else if (
            str_eq(request, "on_a") || str_eq(request, "on_b") ||
            str_eq(request, "offa") || str_eq(request, "offb")) {
            str_cpy(usbmem_response, sizeof(usbmem_response), "done");
        } else if (
            strncmp(request, "lma ", 4) == 0 ||
            strncmp(request, "lmb ", 4) == 0) {
            str_cpy(usbmem_response, sizeof(usbmem_response), "done");
        } else {
            str_cpy(usbmem_response, sizeof(usbmem_response), "not connected");
        }
    }

    usbmem_pending = true;
    src->pos = nbytes;

    return S_OK;
}

static HRESULT usbmem_read(struct irp *irp)
{
    struct iobuf *dest;
    uint32_t rlength;

    log_assert(irp != NULL);
    log_assert(irp->read.bytes != NULL);

    dest = &irp->read;

    if (usbmem_pending) {
        if (strlen(usbmem_response) > 0) {
            log_misc("%s", usbmem_response);
        }

        str_cat(usbmem_response, sizeof(usbmem_response), "\r>");
        usbmem_pending = false;
    }

    rlength = strlen(usbmem_response);
    memcpy(dest->bytes, usbmem_response, rlength);
    memset(usbmem_response, 0, USBMEM_BUF_SIZE);

    dest->pos = rlength;

    return S_OK;
}

static HRESULT usbmem_ioctl(struct irp *irp)
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
            status->AmountInInQueue = usbmem_pending ? 1 : 0;

            irp->read.pos = sizeof(*status);

            break;

        default:
            break;
    }

    return S_OK;
}

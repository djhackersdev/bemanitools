#include <windows.h>

#include <stdbool.h>
#include <stddef.h>

#include "hook/iohook.h"

#include "util/log.h"
#include "util/str.h"

static bool usbmem_match_irp(const struct irp *irp);
static HRESULT usbmem_dispatch_irp_locked(struct irp *irp);
static HRESULT usbmem_handle_open(struct irp *irp);
static HRESULT usbmem_handle_close(struct irp *irp);
static HRESULT usbmem_handle_write(struct irp *irp);
static HRESULT usbmem_handle_read(struct irp *irp);

static CRITICAL_SECTION usbmem_lock;
static HANDLE usbmem_fd;
static bool usbmem_pending;
static char usbmem_response[64];

void usbmem_init(void)
{
    InitializeCriticalSection(&usbmem_lock);
    usbmem_pending = false;
}

void usbmem_fini(void)
{
    struct irp irp;

    if (usbmem_fd != NULL && usbmem_fd != INVALID_HANDLE_VALUE) {
        memset(&irp, 0, sizeof(irp));
        irp.op = IRP_OP_CLOSE;
        irp.fd = usbmem_fd;

        irp_invoke_next(&irp);
    }

    DeleteCriticalSection(&usbmem_lock);
}

HRESULT usbmem_dispatch_irp(struct irp *irp)
{
    HRESULT hr;

    /* usbmem is not performance-critical in any way so we can use coarse
       locking. */

    EnterCriticalSection(&usbmem_lock);

    if (!usbmem_match_irp(irp)) {
        LeaveCriticalSection(&usbmem_lock);

        return irp_invoke_next(irp);
    } else {
        hr = usbmem_dispatch_irp_locked(irp);
        LeaveCriticalSection(&usbmem_lock);

        return hr;
    }
}

static bool usbmem_match_irp(const struct irp *irp)
{
    if (irp->op == IRP_OP_OPEN) {
        return wstr_eq(irp->open_filename, L"COM3");
    } else {
        /* This will not match any IRPs at all unless IRP_OP_OPEN failed and
           we set usbmem_fd to a non-NULL HANDLE (i.e. we started emulation). */
        return irp->fd == usbmem_fd;
    }
}

static HRESULT usbmem_dispatch_irp_locked(struct irp *irp)
{
    switch (irp->op) {
    case IRP_OP_OPEN:   return usbmem_handle_open(irp);
    case IRP_OP_CLOSE:  return usbmem_handle_close(irp);
    case IRP_OP_WRITE:  return usbmem_handle_write(irp);
    case IRP_OP_READ:   return usbmem_handle_read(irp);
    default:            return E_NOTIMPL;
    }
}

static HRESULT usbmem_handle_open(struct irp *irp)
{
    HRESULT hr;

    hr = irp_invoke_next(irp);

    if (SUCCEEDED(hr)) {
        log_info("Opened a real usbmem port");

        return hr;
    }

    log_info("Failed to open real usbmem, will emulate one instead");

    /* Opening the real COM3 port failed. Open a fake FD, return that, and
       start emulating a usbmem unit. */

    if (usbmem_fd == NULL || usbmem_fd == INVALID_HANDLE_VALUE) {
        usbmem_fd = iohook_open_dummy_fd();
    }

    irp->fd = usbmem_fd;

    return S_OK;
}

static HRESULT usbmem_handle_close(struct irp *irp)
{
    usbmem_fd = NULL;

    return irp_invoke_next(irp);
}

static HRESULT usbmem_handle_write(struct irp *irp)
{
    char request[64];
    size_t nbytes;

    nbytes = irp->write.nbytes - irp->write.pos;

    if (nbytes > sizeof(request)) {
        nbytes = sizeof(request);
    }

    memcpy(request, &irp->write.bytes[irp->write.pos], nbytes);
    irp->write.pos += nbytes;

    if (nbytes > 0) {
        request[nbytes - 1] = '\0'; /* This is always a CR. */
    } else {
        request[0] = '\0';          /* Shouldn't ever happen but w/e */
    }

    log_misc(">%s", request);

    if (request[0] != '\0') {
        if (str_eq(request, "sver")) {
            str_cpy(usbmem_response,
                    sizeof(usbmem_response),
                    "done GQHDXJAA UNKNOWN");
        } else if (
                str_eq(request, "on_a") ||
                str_eq(request, "on_b") ||
                str_eq(request, "offa") ||
                str_eq(request, "offb") ||
                strncmp(request, "lma ", 4) == 0 ||
                strncmp(request, "lmb ", 4) == 0 ) {
            str_cpy(usbmem_response, sizeof(usbmem_response), "done");
        } else {
            str_cpy(usbmem_response, sizeof(usbmem_response), "not connected");
        }
    }

    usbmem_pending = true;

    return S_OK;
}

static HRESULT usbmem_handle_read(struct irp *irp)
{
    size_t rlength;

    if (usbmem_pending) {
        if (strlen(usbmem_response) > 0) {
            log_misc("%s", usbmem_response);
        }

        str_cat(usbmem_response, sizeof(usbmem_response), "\r>");
        usbmem_pending = false;
    }

    rlength = strlen(usbmem_response);
    memcpy(&irp->read.bytes[irp->read.pos], usbmem_response, rlength);
    irp->read.pos += rlength;

    memset(usbmem_response, 0, sizeof(usbmem_response));

    return S_OK;
}

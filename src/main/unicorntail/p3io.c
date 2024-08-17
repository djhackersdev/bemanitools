#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "iface-core/log.h"

#include "p3io/cmd.h"
#include "p3io/frame.h"

#include "p3ioemu/uart.h"

#include "unicorntail/p3io.h"

#include "util/array.h"
#include "util/str.h"

static bool p3io_match_irp_locked(const struct irp *irp);

static HRESULT p3io_handle_open(struct irp *irp);
static HRESULT p3io_handle_close(struct irp *irp);
static HRESULT p3io_handle_write(struct irp *irp);
static HRESULT p3io_handle_read(struct irp *irp);

static CRITICAL_SECTION p3io_handle_lock;
static struct array p3io_handles;
static CRITICAL_SECTION p3io_cmd_lock;
static uint8_t p3io_resp_bytes[260];
static struct iobuf p3io_resp;

void p3io_filter_init(void)
{
    InitializeCriticalSection(&p3io_handle_lock);
    InitializeCriticalSection(&p3io_cmd_lock);

    p3io_uart_set_path(0, L"COM4");

    p3io_resp.bytes = p3io_resp_bytes;
    p3io_resp.nbytes = sizeof(p3io_resp_bytes);
    p3io_resp.pos = 0;
}

void p3io_filter_fini(void)
{
    DeleteCriticalSection(&p3io_cmd_lock);
    DeleteCriticalSection(&p3io_handle_lock);
}

HRESULT
p3io_filter_dispatch_irp(struct irp *irp)
{
    bool match;

    EnterCriticalSection(&p3io_handle_lock);
    match = p3io_match_irp_locked(irp);
    LeaveCriticalSection(&p3io_handle_lock);

    if (!match) {
        return iohook_invoke_next(irp);
    }

    switch (irp->op) {
        case IRP_OP_OPEN:
            return p3io_handle_open(irp);
        case IRP_OP_CLOSE:
            return p3io_handle_close(irp);
        case IRP_OP_WRITE:
            return p3io_handle_write(irp);
        case IRP_OP_READ:
            return p3io_handle_read(irp);
        default:
            return iohook_invoke_next(irp);
    }
}

static bool p3io_match_irp_locked(const struct irp *irp)
{
    size_t i;

    if (irp->op == IRP_OP_OPEN) {
        return wstr_ends_with(irp->open_filename, L"\\p3io");
    } else {
        for (i = 0; i < p3io_handles.nitems; i++) {
            if (irp->fd == *array_item(HANDLE, &p3io_handles, i)) {
                return true;
            }
        }

        return false;
    }
}

static HRESULT p3io_handle_open(struct irp *irp)
{
    HRESULT hr;

    hr = iohook_invoke_next(irp);

    if (FAILED(hr)) {
        return hr;
    }

    log_misc("Got p3io fd %p", irp->fd);

    EnterCriticalSection(&p3io_handle_lock);
    *array_append(HANDLE, &p3io_handles) = irp->fd;
    LeaveCriticalSection(&p3io_handle_lock);

    return hr;
}

static HRESULT p3io_handle_close(struct irp *irp)
{
    size_t i;

    log_misc("Releasing p3io fd %p", irp->fd);

    EnterCriticalSection(&p3io_handle_lock);

    for (i = 0; i < p3io_handles.nitems; i++) {
        if (irp->fd == array_item(HANDLE, &p3io_handles, i)) {
            array_remove(HANDLE, &p3io_handles, i);

            break;
        }
    }

    LeaveCriticalSection(&p3io_handle_lock);

    return S_OK;
}

static HRESULT p3io_handle_write(struct irp *irp)
{
    union p3io_req_any req;
    union p3io_resp_any resp;
    struct iobuf desc;
    HRESULT hr;

    desc.bytes = req.raw.data;
    desc.nbytes = sizeof(req);
    desc.pos = 0;

    hr = p3io_frame_decode(&desc, &irp->write);

    if (FAILED(hr)) {
        return hr;
    }

    switch (req.hdr.cmd) {
        case P3IO_CMD_RS232_OPEN_CLOSE:
            EnterCriticalSection(&p3io_cmd_lock);
            p3io_uart_cmd_open_close(
                &req.rs232_open_close, &resp.rs232_open_close);

            break;

        case P3IO_CMD_RS232_WRITE:
            EnterCriticalSection(&p3io_cmd_lock);
            p3io_uart_cmd_write(&req.rs232_write, &resp.rs232_write);

            break;

        case P3IO_CMD_RS232_READ:
            EnterCriticalSection(&p3io_cmd_lock);
            p3io_uart_cmd_read(&req.rs232_read, &resp.rs232_read);

            break;

        default:
            /* Non-UART command, break out here. */
            irp->write.pos = 0;

            return iohook_invoke_next(irp);
    }

    /* Frame up and queue a response packet */

    hr = p3io_frame_encode(&p3io_resp, &resp.hdr, resp.hdr.nbytes + 1);
    LeaveCriticalSection(&p3io_cmd_lock);

    return S_OK;
}

static HRESULT p3io_handle_read(struct irp *irp)
{
    struct const_iobuf tmp;

    EnterCriticalSection(&p3io_cmd_lock);

    if (p3io_resp.pos == 0) {
        LeaveCriticalSection(&p3io_cmd_lock);

        return iohook_invoke_next(irp);
    } else {
        iobuf_flip(&tmp, &p3io_resp);
        iobuf_move(&irp->read, &tmp);
        p3io_resp.pos = 0;

        LeaveCriticalSection(&p3io_cmd_lock);

        return S_OK;
    }
}

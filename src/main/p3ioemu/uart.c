#define LOG_MODULE "p3ioemu-uart"

#include <windows.h>

#include <ntdef.h>
#include <devioctl.h>
#include <ntddser.h>

#include <string.h>

#include "hook/iohook.h"

#include "p3io/cmd.h"

#include "p3ioemu/uart.h"

#include "util/iobuf.h"
#include "util/log.h"

static HRESULT p3io_uart_open(
        const wchar_t *path,
        uint32_t baud_rate,
        HANDLE *fd);
static HRESULT p3io_uart_close(HANDLE fd);
static HRESULT p3io_uart_read(HANDLE fd, struct iobuf *iobuf);
static HRESULT p3io_uart_write(HANDLE fd, struct const_iobuf *iobuf);

static const wchar_t *p3io_uart_paths[2];
static HANDLE p3io_uart_fds[2];

static const uint32_t p3io_uart_baud_codes[] = {
    0, 0, 19200, 38400, 57600
};

void p3io_uart_set_path(size_t uart_no, const wchar_t *path)
{
    log_assert(uart_no <= 1);

    p3io_uart_paths[uart_no] = path;
}

void p3io_uart_cmd_open_close(
        const struct p3io_req_rs232_open_close *req,
        struct p3io_resp_u8 *resp)
{
    const wchar_t *path;
    uint32_t baud_rate;
    HRESULT hr;

    log_assert(req != NULL);
    log_assert(resp != NULL);

    if (req->port_no > 1) {
        log_warning("Invalid UART number %i", req->port_no);
        hr = E_INVALIDARG;

        goto end;
    }

    switch (req->subcmd) {
    case P3IO_RS232_CMD_OPEN:
        log_info("Opening remote RS232 port #%d", req->port_no);

        if (req->baud_code < lengthof(p3io_uart_baud_codes)) {
            baud_rate = p3io_uart_baud_codes[req->baud_code];
        } else {
            baud_rate = 0;
        }

        if (baud_rate == 0) {
            log_warning("Invalid baud rate code: %02x", req->baud_code);
            hr = E_FAIL;

            goto end;
        }

        path = p3io_uart_paths[req->port_no];

        if (path == NULL) {
            log_warning("UART #%i: No downstream connection", req->port_no);
            hr = E_FAIL;

            goto end;
        }

        hr = p3io_uart_open(path, baud_rate, &p3io_uart_fds[req->port_no]);

        if (FAILED(hr)) {
            log_warning("p3io_uart_open() failed: %x", (int) hr);
        }

        break;

    case P3IO_RS232_CMD_CLOSE:
        log_info("Closing remote RS232 port #%d", req->port_no);

        hr = p3io_uart_close(p3io_uart_fds[req->port_no]);
        p3io_uart_fds[req->port_no] = NULL;

        break;

    default:
        log_warning("Unknown subcommand %02x", req->subcmd);
        hr = E_FAIL;

        break;
    }

end:
    p3io_resp_init(&resp->hdr, sizeof(resp), &req->hdr);
    resp->status = 0;
    resp->u8 = FAILED(hr);
}

void p3io_uart_cmd_read(
        const struct p3io_req_rs232_read *req,
        struct p3io_resp_rs232_read *resp)
{
    struct iobuf iobuf;
    HRESULT hr;

    log_assert(req != NULL);
    log_assert(resp != NULL);

    iobuf.bytes = resp->bytes;
    iobuf.nbytes = req->nbytes;
    iobuf.pos = 0;

    if (req->port_no > 1) {
        log_warning("Invalid UART number %i", req->port_no);
        hr = E_INVALIDARG;

        goto end;
    }

    if (req->nbytes > sizeof(resp->bytes)) {
        log_warning("Excessive read %i", req->nbytes);
        hr = E_INVALIDARG;

        goto end;
    }

    hr = p3io_uart_read(p3io_uart_fds[req->port_no], &iobuf);

end:
    /* Variable-length response, init the header manually */

    resp->hdr.nbytes = iobuf.pos + 3;
    resp->hdr.seq_no = req->hdr.seq_no;
    resp->status = FAILED(hr);
    resp->nbytes = iobuf.pos;
}

void p3io_uart_cmd_write(
        const struct p3io_req_rs232_write *req,
        struct p3io_resp_rs232_write *resp)
{
    struct const_iobuf iobuf;
    HRESULT hr;

    iobuf.bytes = req->bytes;
    iobuf.nbytes = req->nbytes;
    iobuf.pos = 0;

    if (req->port_no > 1) {
        log_warning("Invalid UART number %i", req->port_no);
        hr = E_INVALIDARG;

        goto end;
    }

    hr = p3io_uart_write(p3io_uart_fds[req->port_no], &iobuf);

end:
    p3io_resp_init(&resp->hdr, sizeof(resp), &req->hdr);
    resp->status = FAILED(hr);
    resp->nbytes = iobuf.pos;
}

static HRESULT p3io_uart_open(
        const wchar_t *path,
        uint32_t baud_rate,
        HANDLE *fd)
{
    struct irp irp;
    uint32_t comm_mask;
    uint32_t flags;
    SERIAL_QUEUE_SIZE qs;
    SERIAL_TIMEOUTS timeouts;
    SERIAL_LINE_CONTROL lc;
    SERIAL_BAUD_RATE baud;
    SERIAL_HANDFLOW handflow;
    HRESULT hr;

    if (*fd != NULL) {
        log_warning("Port is already open");

        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_OPEN;
    irp.open_filename = path;
    irp.open_access = GENERIC_READ | GENERIC_WRITE;
    irp.open_share = 0;
    irp.open_sa = NULL;
    irp.open_creation = OPEN_EXISTING;
    irp.open_flags = 0;
    irp.open_tmpl = NULL;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    *fd = irp.fd;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = *fd;
    irp.ioctl = IOCTL_SERIAL_SET_WAIT_MASK;
    irp.write.bytes = (const void *) &comm_mask;
    irp.write.nbytes = sizeof(comm_mask);
    comm_mask = EV_RXCHAR;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = *fd;
    irp.ioctl = IOCTL_SERIAL_SET_QUEUE_SIZE;
    irp.write.bytes = (const void *) &qs;
    irp.write.nbytes = sizeof(qs);
    qs.InSize = 0x4000;
    qs.OutSize = 0x4000;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = *fd;
    irp.ioctl = IOCTL_SERIAL_PURGE;
    irp.write.bytes = (const void *) &flags;
    irp.write.nbytes = sizeof(flags);
    flags = PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = *fd;
    irp.ioctl = IOCTL_SERIAL_SET_TIMEOUTS;
    irp.write.bytes = (const void *) &timeouts;
    irp.write.nbytes = sizeof(timeouts);
    timeouts.ReadIntervalTimeout = -1;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 100;
    timeouts.WriteTotalTimeoutConstant = 0;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = *fd;
    irp.ioctl = IOCTL_SERIAL_SET_LINE_CONTROL;
    irp.write.bytes = (const void *) &lc;
    irp.write.nbytes = sizeof(lc);
    lc.WordLength = 8;
    lc.Parity = NO_PARITY;
    lc.StopBits = STOP_BIT_1;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = *fd;
    irp.ioctl = IOCTL_SERIAL_SET_BAUD_RATE;
    irp.write.bytes = (const void *) &baud;
    irp.write.nbytes = sizeof(baud);
    baud.BaudRate = baud_rate;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = *fd;
    irp.ioctl = IOCTL_SERIAL_SET_HANDFLOW;
    irp.write.bytes = (const void *) &handflow;
    irp.write.nbytes = sizeof(handflow);
    handflow.ControlHandShake = 0;
    handflow.FlowReplace = 0;
    handflow.XonLimit = 0;
    handflow.XoffLimit = 0;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        goto fail;
    }

    return S_OK;

fail:
    if (*fd != NULL) {
        irp.op = IRP_OP_CLOSE;
        irp.fd = *fd;

        irp_invoke_next(&irp);

        *fd = NULL;
    }

    return hr;
}

static HRESULT p3io_uart_close(HANDLE fd)
{
    struct irp irp;
    HRESULT hr;

    if (fd != NULL) {
        memset(&irp, 0, sizeof(irp));
        irp.op = IRP_OP_CLOSE;
        irp.fd = fd;

        hr = irp_invoke_next(&irp);

        if (FAILED(hr)) {
            log_warning("Error closing port: %x", (int) hr);
        }
    } else {
        log_warning("Port is already closed");
        hr = S_OK;
    }

    return hr;
}

static HRESULT p3io_uart_read(HANDLE fd, struct iobuf *iobuf)
{
    struct irp irp;
    SERIAL_STATUS status;
    HRESULT hr;

    if (fd == NULL) {
        log_warning("Read from unopened port");

        return E_FAIL;
    }

    /* Peek RX buffer */

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_GET_COMMSTATUS;
    irp.read.bytes = (uint8_t *) &status;
    irp.read.nbytes = sizeof(status);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("UART FIFO peek failed: %x", (int) hr);

        return hr;
    }

    /* Return immediately if no data available */

    if (status.AmountInInQueue == 0) {
        return S_OK;
    }

    /* Issue read */

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_READ;
    irp.fd = fd;
    memcpy(&irp.read, iobuf, sizeof(*iobuf));

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("Read error: %x", (int) hr);

        return hr;
    }

    memcpy(iobuf, &irp.read, sizeof(*iobuf));

    return S_OK;
}

static HRESULT p3io_uart_write(HANDLE fd, struct const_iobuf *iobuf)
{
    struct irp irp;
    HRESULT hr;

    if (fd == NULL) {
        log_warning("Write to unopened port");

        return E_FAIL;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_WRITE;
    irp.fd = fd;
    memcpy(&irp.write, iobuf, sizeof(*iobuf));

    hr = irp_invoke_next(&irp);

    if (SUCCEEDED(hr)) {
        memcpy(iobuf, &irp.write, sizeof(*iobuf));
    } else {
        log_warning("Write error: %x", (int) hr);
    }

    return hr;
}

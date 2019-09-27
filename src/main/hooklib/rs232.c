#include <windows.h>    /* Usermode API */

#include <ntdef.h>      /* Kernel-mode API for ioctls */
#include <devioctl.h>
#include <ntddser.h>

#include <string.h>

#include "hook/iohook.h"
#include "hook/table.h"

#include "util/hr.h"
#include "util/log.h"

/* RS232 API hooks */

static BOOL STDCALL my_ClearCommError(
        HANDLE fd,
        uint32_t *errors,
        COMSTAT *status);

static BOOL STDCALL my_EscapeCommFunction(HANDLE fd, uint32_t func);
static BOOL STDCALL my_GetCommState(HANDLE fd, DCB *dcb);
static BOOL STDCALL my_PurgeComm(HANDLE fd, uint32_t flags);
static BOOL STDCALL my_SetCommMask(HANDLE fd, uint32_t mask);
static BOOL STDCALL my_SetCommState(HANDLE fd, const DCB *dcb);
static BOOL STDCALL my_SetCommTimeouts(HANDLE fd, COMMTIMEOUTS *timeouts);
static BOOL STDCALL my_SetupComm(HANDLE fd, uint32_t in_q, uint32_t out_q);
static BOOL STDCALL my_SetCommBreak(HANDLE fd);
static BOOL STDCALL my_ClearCommBreak(HANDLE fd);

static struct hook_symbol rs232_syms[] = {
    {
        .name   = "ClearCommError",
        .patch  = my_ClearCommError,
    },
    {
        .name   = "EscapeCommFunction",
        .patch  = my_EscapeCommFunction,
    },
    {
        .name   = "GetCommState",
        .patch  = my_GetCommState,
    },
    {
        .name   = "PurgeComm",
        .patch  = my_PurgeComm,
    },
    {
        .name   = "SetCommMask",
        .patch  = my_SetCommMask,
    },
    {
        .name   = "SetCommState",
        .patch  = my_SetCommState,
    },
    {
        .name   = "SetCommTimeouts",
        .patch  = my_SetCommTimeouts,
    },
    {
        .name   = "SetupComm",
        .patch  = my_SetupComm,
    },
    {
        .name   = "SetCommBreak",
        .patch  = my_SetCommBreak,
    },
    {
        .name   = "ClearCommBreak",
        .patch  = my_ClearCommBreak,
    },
};

void rs232_hook_init(void)
{
    hook_table_apply(NULL, "kernel32.dll", rs232_syms, lengthof(rs232_syms));
    log_info("IO Hook RS232 ioctl subsystem initialized");
}

static BOOL STDCALL my_ClearCommError(
        HANDLE fd,
        uint32_t *errors,
        COMSTAT *status)
{
    struct irp irp;
    SERIAL_STATUS llstatus;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_GET_COMMSTATUS;
    irp.read.bytes = (uint8_t *) &llstatus;
    irp.read.nbytes = sizeof(llstatus);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning(
                "%s: IOCTL_SERIAL_GET_COMMSTATUS failed: %lx",
                __func__,
                hr);

        return hr_propagate_win32(hr, FALSE);
    }

    /* Here we just translate between two structures that carry essentially the
       same information, because Windows. */

    if (errors != NULL) {
        *errors = 0;

        if (llstatus.Errors & SERIAL_ERROR_QUEUEOVERRUN) {
            *errors |= CE_OVERRUN;
        }

        if (llstatus.Errors & SERIAL_ERROR_OVERRUN) {
            *errors |= CE_RXOVER;
        }

        if (llstatus.Errors & SERIAL_ERROR_BREAK) {
            *errors |= CE_BREAK;
        }

        if (llstatus.Errors & SERIAL_ERROR_PARITY) {
            *errors |= CE_RXPARITY;
        }

        if (llstatus.Errors & SERIAL_ERROR_FRAMING) {
            *errors |= CE_FRAME;
        }
    }

    if (status != NULL) {
        memset(status, 0, sizeof(*status));

        if (llstatus.HoldReasons & SERIAL_TX_WAITING_FOR_CTS) {
            status->fCtsHold = 1;
        }

        if (llstatus.HoldReasons & SERIAL_TX_WAITING_FOR_DSR) {
            status->fDsrHold = 1;
        }

        if (llstatus.HoldReasons & SERIAL_TX_WAITING_FOR_DCD) {
            status->fRlsdHold = 1;
        }

        if (llstatus.HoldReasons & SERIAL_TX_WAITING_FOR_XON) {
            status->fXoffHold = 1;
        }

        if (llstatus.HoldReasons & SERIAL_TX_WAITING_ON_BREAK) {
            /* hrm. No corresponding (documented field). */
        }

        if (llstatus.HoldReasons & SERIAL_TX_WAITING_XOFF_SENT) {
            status->fXoffSent = 1;
        }

        if (llstatus.EofReceived) {
            status->fEof = 1;
        }

        if (llstatus.WaitForImmediate) {
            status->fTxim = 1;
        }

        status->cbInQue = llstatus.AmountInInQueue;
        status->cbOutQue = llstatus.AmountInOutQueue;
    }

    return TRUE;
}

static BOOL STDCALL my_EscapeCommFunction(HANDLE fd, uint32_t cmd)
{
    struct irp irp;
    uint32_t ioctl;
    HRESULT hr;

    switch (cmd) {
    case CLRBREAK:  ioctl = IOCTL_SERIAL_SET_BREAK_OFF; break;
    case CLRDTR:    ioctl = IOCTL_SERIAL_CLR_DTR; break;
    case CLRRTS:    ioctl = IOCTL_SERIAL_CLR_RTS; break;
    case SETBREAK:  ioctl = IOCTL_SERIAL_SET_BREAK_ON; break;
    case SETDTR:    ioctl = IOCTL_SERIAL_SET_RTS; break;
    case SETRTS:    ioctl = IOCTL_SERIAL_SET_RTS; break;
    case SETXOFF:   ioctl = IOCTL_SERIAL_SET_XOFF; break;
    case SETXON:    ioctl = IOCTL_SERIAL_SET_XON; break;
    default:
        log_warning("%s: Invalid comm function %08x", __func__, cmd);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = ioctl;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: ioctl failed: %lx", __func__, hr);

        return hr_propagate_win32(hr, FALSE);
    }

    return TRUE;
}

static BOOL STDCALL my_GetCommState(HANDLE fd, DCB *dcb)
{
    struct irp irp;
    SERIAL_BAUD_RATE baud;
    SERIAL_CHARS chars;
    SERIAL_HANDFLOW handflow;
    SERIAL_LINE_CONTROL line_control;
    HRESULT hr;

    /*
     * Validate params
     */

    if (dcb == NULL) {
        log_warning("%s: DCB pointer is NULL", __func__);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    if (dcb->DCBlength != sizeof(*dcb)) {
        /* This struct has evolved in the past, but those were the Windows 95
           days. So we only support the latest size of this struct. */

        log_warning(
                "%s: dcb->DCBlength = %d, expected %d",
                __func__,
                (int) dcb->DCBlength,
                (unsigned int) sizeof(*dcb));
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    /*
     * Issue ioctls
     */

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_GET_BAUD_RATE;
    irp.read.bytes = (uint8_t *) &baud;
    irp.read.nbytes = sizeof(baud);
    memset(&baud, 0, sizeof(baud));

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_GET_BAUD_RATE failed: %lx", __func__, hr);

        return hr_propagate_win32(hr, FALSE);
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_GET_HANDFLOW;
    irp.read.bytes = (uint8_t *) &handflow;
    irp.read.nbytes = sizeof(handflow);
    memset(&handflow, 0, sizeof(handflow));

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_GET_HANDFLOW failed: %lx", __func__, hr);

        return hr_propagate_win32(hr, FALSE);
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_GET_LINE_CONTROL;
    irp.read.bytes = (uint8_t *) &line_control;
    irp.read.nbytes = sizeof(line_control);
    memset(&line_control, 0, sizeof(line_control));

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning(
                "%s: IOCTL_SERIAL_GET_LINE_CONTROL failed: %lx",
                __func__,
                hr);

        return hr_propagate_win32(hr, FALSE);
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_GET_CHARS;
    irp.read.bytes = (uint8_t *) &chars;
    irp.read.nbytes = sizeof(chars);
    memset(&chars, 0, sizeof(chars));

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_GET_CHARS failed: %lx", __func__, hr);

        return hr_propagate_win32(hr, FALSE);
    }

    /*
     * Populate output struct
     */

    memset(dcb, 0, sizeof(*dcb));
    dcb->DCBlength = sizeof(*dcb);
    dcb->fBinary = 1;
    dcb->BaudRate = baud.BaudRate;

    /* Populate fParity somehow? */

    if (handflow.ControlHandShake & SERIAL_CTS_HANDSHAKE) {
        dcb->fOutxCtsFlow = 1;
    }

    if (handflow.ControlHandShake & SERIAL_DSR_HANDSHAKE) {
        dcb->fOutxDsrFlow = 1;
    }

    if (handflow.ControlHandShake & SERIAL_DTR_CONTROL) {
        dcb->fDtrControl = DTR_CONTROL_ENABLE;
    }

    if (handflow.ControlHandShake & SERIAL_DTR_HANDSHAKE) {
        dcb->fDtrControl = DTR_CONTROL_HANDSHAKE;
    }

    if (handflow.ControlHandShake & SERIAL_DSR_SENSITIVITY) {
        dcb->fDsrSensitivity = 1;
    }

    if (handflow.ControlHandShake & SERIAL_XOFF_CONTINUE) {
        dcb->fTXContinueOnXoff = 1;
    }

    if (handflow.ControlHandShake & SERIAL_RTS_CONTROL) {
        dcb->fRtsControl = RTS_CONTROL_ENABLE;
    }

    if (handflow.ControlHandShake & SERIAL_RTS_HANDSHAKE) {
        dcb->fRtsControl = RTS_CONTROL_HANDSHAKE;
    }

    if (handflow.ControlHandShake & SERIAL_ERROR_ABORT) {
        dcb->fAbortOnError = 1;
    }

    if (handflow.ControlHandShake & SERIAL_ERROR_CHAR) {
        dcb->fErrorChar = 1;
    }

    if (handflow.ControlHandShake & SERIAL_NULL_STRIPPING) {
        dcb->fNull = 1;
    }

    dcb->XonLim = handflow.XonLimit;
    dcb->XoffLim = handflow.XoffLimit;
    dcb->ByteSize = line_control.WordLength;
    dcb->Parity = line_control.Parity;
    dcb->StopBits = line_control.StopBits;
    dcb->XonChar = chars.XonChar;
    dcb->XoffChar = chars.XoffChar;
    dcb->ErrorChar = chars.ErrorChar;
    dcb->EofChar = chars.EofChar;
    dcb->EvtChar = chars.EventChar;

    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL STDCALL my_PurgeComm(HANDLE fd, uint32_t flags)
{
    struct irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_PURGE;
    irp.write.bytes = (uint8_t *) &flags;
    irp.write.nbytes = sizeof(flags);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_PURGE failed: %lx", __func__, hr);
    }

    return hr_propagate_win32(hr, SUCCEEDED(hr));
}


static BOOL STDCALL my_SetCommMask(HANDLE fd, uint32_t mask)
{
    struct irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_WAIT_MASK;
    irp.write.bytes = (uint8_t *) &mask;
    irp.write.nbytes = sizeof(mask);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_WAIT_MASK failed: %lx", __func__, hr);
    }

    return hr_propagate_win32(hr, SUCCEEDED(hr));
}

static BOOL STDCALL my_SetCommState(HANDLE fd, const DCB *dcb)
{
    struct irp irp;
    SERIAL_BAUD_RATE baud;
    SERIAL_CHARS chars;
    SERIAL_HANDFLOW handflow;
    SERIAL_LINE_CONTROL line_control;
    HRESULT hr;

    if (dcb == NULL) {
        log_warning("%s: DCB pointer is NULL", __func__);
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    if (dcb->DCBlength != sizeof(*dcb)) {
        /* This struct has evolved in the past, but those were the Windows 95
           days. So we only support the latest size of this struct. */

        log_warning(
                "%s: dcb->DCBlength = %d, expected %d",
                __func__,
                (int) dcb->DCBlength,
                (unsigned int) sizeof(*dcb));
        SetLastError(ERROR_INVALID_PARAMETER);

        return FALSE;
    }

    memset(&baud, 0, sizeof(baud));
    baud.BaudRate = dcb->BaudRate;

    memset(&handflow, 0, sizeof(handflow));

    if (dcb->fOutxCtsFlow) {
        handflow.ControlHandShake |= SERIAL_CTS_HANDSHAKE;
    }

    if (dcb->fOutxDsrFlow) {
        handflow.ControlHandShake |= SERIAL_DSR_HANDSHAKE;
    }

    switch (dcb->fDtrControl) {
        case DTR_CONTROL_DISABLE:
            break;

        case DTR_CONTROL_ENABLE:
            handflow.ControlHandShake |= SERIAL_DTR_CONTROL;

            break;

        case DTR_CONTROL_HANDSHAKE:
            handflow.ControlHandShake |= SERIAL_DTR_HANDSHAKE;

            break;

        default:
            log_warning("%s: dcb->fDtrControl value %d is invalid",
                    __func__, dcb->fDtrControl);
            SetLastError(ERROR_INVALID_PARAMETER);

            return FALSE;
    }

    if (dcb->fDsrSensitivity) {
        handflow.ControlHandShake |= SERIAL_DSR_SENSITIVITY;
    }

    if (dcb->fTXContinueOnXoff) {
        handflow.ControlHandShake |= SERIAL_XOFF_CONTINUE;
    }

    switch (dcb->fRtsControl) {
        case RTS_CONTROL_DISABLE:
            break;

        case RTS_CONTROL_ENABLE:
            handflow.ControlHandShake |= SERIAL_RTS_CONTROL;

            break;

        case RTS_CONTROL_HANDSHAKE:
            handflow.ControlHandShake |= SERIAL_RTS_HANDSHAKE;

            break;

        default:
            log_warning(
                    "%s: dcb->fRtsControl value %d is invalid",
                    __func__,
                    dcb->fRtsControl);
            SetLastError(ERROR_INVALID_PARAMETER);

            return FALSE;
    }

    memset(&line_control, 0, sizeof(line_control));
    line_control.WordLength = dcb->ByteSize;
    line_control.Parity = dcb->Parity;
    line_control.StopBits = dcb->StopBits;

    memset(&chars, 0, sizeof(chars));
    chars.XonChar = dcb->XonChar;
    chars.XoffChar = dcb->XoffChar;
    chars.ErrorChar = dcb->ErrorChar;
    chars.EofChar = dcb->EofChar;
    chars.EventChar = dcb->EvtChar;

    /* Parameters populated and validated, commit new settings */

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_BAUD_RATE;
    irp.write.bytes = (uint8_t *) &baud;
    irp.write.nbytes = sizeof(baud);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_BAUD_RATE failed: %lx", __func__, hr);

        return hr_propagate_win32(hr, FALSE);
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_HANDFLOW;
    irp.write.bytes = (uint8_t *) &handflow;
    irp.write.nbytes = sizeof(handflow);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_HANDFLOW failed: %lx", __func__, hr);

        return hr_propagate_win32(hr, FALSE);
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_LINE_CONTROL;
    irp.write.bytes = (uint8_t *) &line_control;
    irp.write.nbytes = sizeof(line_control);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning(
                "%s: IOCTL_SERIAL_SET_LINE_CONTROL failed: %lx",
                __func__,
                hr);

        return hr_propagate_win32(hr, FALSE);
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_CHARS;
    irp.write.bytes = (uint8_t *) &chars;
    irp.write.nbytes = sizeof(chars);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_CHARS failed: %lx", __func__, hr);

        return hr_propagate_win32(hr, FALSE);
    }

    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL STDCALL my_SetCommTimeouts(HANDLE fd, COMMTIMEOUTS *src)
{
    struct irp irp;
    SERIAL_TIMEOUTS dest;
    HRESULT hr;

    dest.ReadIntervalTimeout            = src->ReadIntervalTimeout;
    dest.ReadTotalTimeoutMultiplier     = src->ReadTotalTimeoutMultiplier;
    dest.ReadTotalTimeoutConstant       = src->ReadTotalTimeoutConstant;
    dest.WriteTotalTimeoutMultiplier    = src->WriteTotalTimeoutMultiplier;
    dest.WriteTotalTimeoutConstant      = src->WriteTotalTimeoutConstant;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_TIMEOUTS;
    irp.write.bytes = (uint8_t *) &dest;
    irp.write.nbytes = sizeof(dest);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_TIMEOUTS failed: %lx", __func__, hr);
    }

    return hr_propagate_win32(hr, SUCCEEDED(hr));
}

static BOOL STDCALL my_SetupComm(HANDLE fd, uint32_t in_q, uint32_t out_q)
{
    struct irp irp;
    SERIAL_QUEUE_SIZE qs;
    HRESULT hr;

    qs.InSize = in_q;
    qs.OutSize = out_q;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_QUEUE_SIZE;
    irp.write.bytes = (uint8_t *) &qs;
    irp.write.nbytes = sizeof(qs);

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_QUEUE_SIZE failed: %lx",__func__, hr);
    }

    return hr_propagate_win32(hr, SUCCEEDED(hr));
}

static BOOL STDCALL my_SetCommBreak(HANDLE fd)
{
    struct irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_BREAK_ON;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_BREAK_ON failed: %lx", __func__, hr);
    }

    return hr_propagate_win32(hr, SUCCEEDED(hr));
}

static BOOL STDCALL my_ClearCommBreak(HANDLE fd)
{
    struct irp irp;
    HRESULT hr;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_IOCTL;
    irp.fd = fd;
    irp.ioctl = IOCTL_SERIAL_SET_BREAK_OFF;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_warning("%s: IOCTL_SERIAL_SET_BREAK_OFF failed: %lx", __func__, hr);
    }

    return hr_propagate_win32(hr, SUCCEEDED(hr));
}


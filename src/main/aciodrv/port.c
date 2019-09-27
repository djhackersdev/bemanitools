#define LOG_MODULE "aciodrv-port"

#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#include "util/log.h"

static HANDLE aciodrv_port_fd;

bool aciodrv_port_open(const char* port, int baud)
{
    COMMTIMEOUTS ct;
    DCB dcb;

    log_info("Opening ACIO on %s at %d baud", port, baud);

    aciodrv_port_fd = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_NORMAL, NULL);

    if (aciodrv_port_fd == INVALID_HANDLE_VALUE) {
        log_warning("Failed to open %s", port);

        goto early_fail;
    }

    if (!SetCommMask(aciodrv_port_fd, EV_RXCHAR)) {
        log_warning("SetCommMask failed");

        goto fail;
    }

    if (!SetupComm(aciodrv_port_fd, 0x1000, 0x1000)) {
        log_warning("SetupComm failed");

        goto fail;
    }

    if (!PurgeComm(aciodrv_port_fd,
        PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {

        log_warning("PurgeComm failed");

        goto fail;
    }

    ct.ReadTotalTimeoutConstant = 0;
    ct.WriteTotalTimeoutConstant = 0;
    ct.ReadIntervalTimeout = -1;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(aciodrv_port_fd, &ct)) {
        log_warning("SetCommTimeouts failed");

        goto fail;
    }

    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(aciodrv_port_fd, &dcb)) {
        log_warning("GetCommState failed");

        goto fail;
    }

    dcb.BaudRate = baud;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fAbortOnError = FALSE;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.XonChar = 17;
    dcb.XoffChar = 19;
    dcb.XonLim = 100;
    dcb.XoffLim = 100;

    if (!SetCommState(aciodrv_port_fd, &dcb)) {
        log_warning("SetCommState failed");

        goto fail;
    }

    if (!EscapeCommFunction(aciodrv_port_fd, SETDTR)) {
        log_warning("SETDTR failed: err = %lu", GetLastError());

        goto fail;
    }

    log_info("Opened ACIO device on %s", port);

    return true;

fail:
    CloseHandle(aciodrv_port_fd);

early_fail:
    aciodrv_port_fd = NULL;
    return false;
}

int aciodrv_port_read(void *bytes, int nbytes)
{
    DWORD nread;

    if (aciodrv_port_fd == NULL) {
        return -1;
    }

    if (!ClearCommError(aciodrv_port_fd, NULL, NULL)) {
        log_warning("ClearCommError failed");

        return -1;
    }

    if (!ReadFile(aciodrv_port_fd, bytes, nbytes, &nread, NULL)) {
        log_warning("ReadFile failed: err = %lu", GetLastError());

        return -1;
    }

    return nread;
}

int aciodrv_port_write(const void *bytes, int nbytes)
{
    DWORD nwrit;

    if (aciodrv_port_fd == NULL) {
        return -1;
    }

    if (!WriteFile(aciodrv_port_fd, bytes, nbytes, &nwrit, NULL)) {
        log_warning("WriteFile failed: err = %lu", GetLastError());

        return -1;
       }

    return nwrit;
}

void aciodrv_port_close(void)
{
    if (aciodrv_port_fd != NULL) {
        CloseHandle(aciodrv_port_fd);
    }
}


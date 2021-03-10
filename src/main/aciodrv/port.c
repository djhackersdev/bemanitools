#define LOG_MODULE "aciodrv-port"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#include "util/log.h"

HANDLE aciodrv_port_open(const char *port_path, int baud)
{
    COMMTIMEOUTS ct;
    DCB dcb;

    log_info("Opening ACIO on %s at %d baud", port_path, baud);

    HANDLE port_fd = CreateFile(
        port_path,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (port_fd == INVALID_HANDLE_VALUE) {
        log_warning("Failed to open %s", port_path);

        goto early_fail;
    }

    if (!SetCommMask(port_fd, EV_RXCHAR)) {
        log_warning("SetCommMask failed");

        goto fail;
    }

    if (!SetupComm(port_fd, 0x1000, 0x1000)) {
        log_warning("SetupComm failed");

        goto fail;
    }

    if (!PurgeComm(
            port_fd,
            PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
        log_warning("PurgeComm failed");

        goto fail;
    }

    ct.ReadTotalTimeoutConstant = 0;
    ct.WriteTotalTimeoutConstant = 0;
    ct.ReadIntervalTimeout = -1;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(port_fd, &ct)) {
        log_warning("SetCommTimeouts failed");

        goto fail;
    }

    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(port_fd, &dcb)) {
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

    if (!SetCommState(port_fd, &dcb)) {
        log_warning("SetCommState failed");

        goto fail;
    }

    if (!EscapeCommFunction(port_fd, SETDTR)) {
        log_warning("SETDTR failed: err = %lu", GetLastError());

        goto fail;
    }

    log_info("[%" PRIXPTR "] Opened ACIO device on %s", (uintptr_t)port_fd, port_path);

    return port_fd;

fail:
    CloseHandle(port_fd);

early_fail:
    return NULL;
}

int aciodrv_port_read(HANDLE port_fd, void *bytes, int nbytes)
{
    DWORD nread;

    log_assert(bytes);

    if (port_fd == NULL) {
        return -1;
    }

    if (!ClearCommError(port_fd, NULL, NULL)) {
        log_warning("[%" PRIXPTR "] ClearCommError failed", (uintptr_t)port_fd);

        return -1;
    }

    if (!ReadFile(port_fd, bytes, nbytes, &nread, NULL)) {
        log_warning("[%" PRIXPTR "] ReadFile failed: err = %lu", (uintptr_t)port_fd, GetLastError());

        return -1;
    }

    return nread;
}

int aciodrv_port_write(HANDLE port_fd, const void *bytes, int nbytes)
{
    DWORD nwrit;

    log_assert(bytes);

    if (port_fd == NULL) {
        return -1;
    }

    if (!WriteFile(port_fd, bytes, nbytes, &nwrit, NULL)) {
        log_warning("[%" PRIXPTR "] WriteFile failed: err = %lu", (uintptr_t)port_fd, GetLastError());

        return -1;
    }

    return nwrit;
}

void aciodrv_port_close(HANDLE port_fd)
{
    if (port_fd != NULL) {
        CloseHandle(port_fd);
    }
}

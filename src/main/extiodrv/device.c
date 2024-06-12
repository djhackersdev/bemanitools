#define LOG_MODULE "extiodrv-device"

#include "device.h"

#include "iface-core/log.h"

HRESULT extiodrv_device_open(const char *port, HANDLE *handle)
{
    HRESULT hr;
    COMMTIMEOUTS ct;
    DCB dcb;

    log_assert(port);
    log_assert(handle);

    log_info("Opening extio on %s", port);

    *handle = CreateFile(
        port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (*handle == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("Failed to open %s: %lX", port, hr);

        goto early_fail;
    }

    if (!SetCommMask(*handle, EV_RXCHAR)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("SetCommMask failed: %lX", hr);

        goto fail;
    }

    if (!SetupComm(*handle, 0x4000u, 0x4000u)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("SetupComm failed: %lX", hr);

        goto fail;
    }

    if (!PurgeComm(
            *handle,
            PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("PurgeComm failed: %lX", hr);

        goto fail;
    }

    ct.ReadTotalTimeoutConstant = 0;
    ct.WriteTotalTimeoutConstant = 0;
    ct.ReadIntervalTimeout = -1;
    ct.ReadTotalTimeoutMultiplier = 10;
    ct.WriteTotalTimeoutMultiplier = 100;

    if (!SetCommTimeouts(*handle, &ct)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("SetCommTimeouts failed: %lX", hr);

        goto fail;
    }

    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(*handle, &dcb)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("GetCommState failed: %lX", hr);

        goto fail;
    }

    dcb.BaudRate = 38400;
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

    if (!SetCommState(*handle, &dcb)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("SetCommState failed: %lX", hr);

        goto fail;
    }

    log_info("[%p] Opened extio device on %s", *handle, port);

    return S_OK;

fail:
    CloseHandle(*handle);
    *handle = INVALID_HANDLE_VALUE;

early_fail:
    return hr;
}

HRESULT extiodrv_device_close(HANDLE *handle)
{
    HRESULT hr;

    log_assert(handle);

    if (CloseHandle(*handle) == FALSE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    *handle = INVALID_HANDLE_VALUE;

    return S_OK;
}

HRESULT extiodrv_device_read(
    HANDLE handle, void *bytes, size_t nbytes, size_t *read_bytes)
{
    HRESULT hr;
    DWORD dw_read_bytes;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(bytes);
    log_assert(read_bytes);

    if (!ClearCommError(handle, NULL, NULL)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("[%p] ClearCommError failed: %lX", handle, hr);

        return hr;
    }

    if (!ReadFile(handle, bytes, nbytes, &dw_read_bytes, NULL)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("[%p] ReadFile failed: %lX", handle, hr);

        return hr;
    }

    *read_bytes = dw_read_bytes;

    return S_OK;
}

HRESULT extiodrv_device_write(
    HANDLE handle, const void *bytes, size_t nbytes, size_t *written_bytes)
{
    HRESULT hr;
    DWORD dw_written_bytes;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(bytes);
    log_assert(written_bytes);

    if (!WriteFile(handle, bytes, nbytes, &dw_written_bytes, NULL)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("[%p] WriteFile failed: %lX", handle, hr);

        return hr;
    }

    *written_bytes = dw_written_bytes;

    return S_OK;
}
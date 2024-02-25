#define LOG_MODULE "p3iodrv-device"

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
// clang-format on

#include "core/log.h"

#include "p3io/cmd.h"
#include "p3io/guid.h"
#include "p3io/ioctl.h"

#include "p3iodrv/device.h"

#include "util/str.h"

#define P3IO_DEVICE_FILENMAME "\\p3io"

static HRESULT _p3iodrv_write_file_iobuf(HANDLE handle, struct iobuf *buffer)
{
    HRESULT res;
    DWORD bytes_processed;

    if (!WriteFile(
            handle, buffer->bytes, buffer->pos, &bytes_processed, NULL)) {
        res = HRESULT_FROM_WIN32(GetLastError());
        log_warning("WriteFile failed: %lX", res);
        return res;
    }

    log_misc("Written length: %ld", bytes_processed);

    if (bytes_processed != buffer->pos) {
        log_warning(
            "WriteFile didn't finish: %ld != %Id",
            bytes_processed,
            buffer->pos);
        return HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
    }

    return S_OK;
}

static HRESULT _p3iodrv_read_file_iobuf(HANDLE handle, struct iobuf *buffer)
{
    HRESULT hr;
    DWORD bytes_processed;

    if (!ReadFile(
            handle,
            buffer->bytes + buffer->pos,
            buffer->nbytes - buffer->pos,
            &bytes_processed,
            NULL)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        log_warning("ReadFile failed: %lX", hr);
        return hr;
    }

    log_misc("Read length: %ld", bytes_processed);

    buffer->pos += bytes_processed;

    return S_OK;
}

static HRESULT
_p3iodrv_write_message(HANDLE handle, const union p3io_req_any *req)
{
    HRESULT hr;
    uint8_t req_framed_bytes[P3IO_MAX_MESSAGE_SIZE];

    struct iobuf req_deframed;
    struct iobuf req_framed;

    memset(req_framed_bytes, 0, sizeof(req_framed_bytes));

    // Used for logging the buffer, only
    req_deframed.bytes = (uint8_t *) req;
    req_deframed.nbytes = sizeof(union p3io_req_any);
    req_deframed.pos = req->hdr.nbytes + 1;

    req_framed.bytes = req_framed_bytes;
    req_framed.nbytes = sizeof(req_framed_bytes);
    req_framed.pos = 0;

    iobuf_log(&req_deframed, "p3iodrv-device request deframed");

    hr = p3io_frame_encode(&req_framed, req, req->hdr.nbytes + 1);

    if (FAILED(hr)) {
        log_warning("Encoding request payload failed: %lX", hr);
        return hr;
    }

    iobuf_log(&req_framed, "p3iodrv-device request framed");

    hr = _p3iodrv_write_file_iobuf(handle, &req_framed);

    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

/**
 * Read a full response message and not just a part of it.
 *
 * Because someone decided to implement a general "streaming bytes" solution
 * with framing (ACIO) on top of a streaming byte solution with framing (USB),
 * there is no guarantee that single reads are complete. This even applies to
 * reads that are less then the max USB endpoint size.
 *
 * When polling to read data too fast in succession to writes, e.g. on a p3io
 * command which first writes a request, then reads a response, the p3io
 * hardware might not be fast enough to put all bytes of the full response into
 * the device hardware side buffer. It appears that the device hardware/the
 * firmware pushes out chunks of/single bytes instead of preparing full messages
 * that are framed to the available USB buffer size.
 *
 * Therefore, messages can get fragmented and require multiple read calls to
 * ensure the read buffer is fully flushed on the device side and received on
 * the host. The following solution does exactly that it "reads until read
 * everything". The following heuristics are applied to determine when we think
 * a response message is fully received:
 *
 * 1. Read until you get parts of a header to determine how long the entire ACIO
 * message is supposed to be. This must assume that the device will always put
 * the correct amount of bytes onto the wire at some point. If it stopps doing
 * that, there is no way to possibility to detect and take action on that.
 * 2. Read until you get the complete ACIO paket based on the ACIO paket length
 * field.
 */
static HRESULT
_p3iodrv_read_full_response_message(HANDLE handle, union p3io_resp_any *resp)
{
    HRESULT hr;
    uint8_t resp_framed_bytes[P3IO_MAX_MESSAGE_SIZE];

    struct iobuf resp_framed;
    struct iobuf resp_framed_flip_read;
    struct iobuf resp_deframed;

    memset(resp_framed_bytes, 0, sizeof(resp_framed_bytes));

    resp_framed.bytes = resp_framed_bytes;
    resp_framed.nbytes = sizeof(resp_framed_bytes);
    resp_framed.pos = 0;

    log_misc("Receiving response");

    // Read as long as we do not consider the message to be complete
    while (true) {
        hr = _p3iodrv_read_file_iobuf(handle, &resp_framed);

        if (FAILED(hr)) {
            return hr;
        }

        // Read at least 0xAA + header size, otherwise keep reading
        if (resp_framed.pos < 1 + sizeof(struct p3io_hdr)) {
            log_misc(
                "Read truncated message, length %ld less than header, read "
                "again",
                (DWORD) resp_framed.pos);
            continue;
        }

        log_misc("Response received, length: %ld", (DWORD) resp_framed.pos);

        iobuf_log(&resp_framed, "p3iodrv-device response framed");

        // Potential pre-mature deframing because we cannot be sure we already
        // received the entire frame

        // Use a flipped buffer to read from the framed response
        resp_framed_flip_read.bytes = resp_framed.bytes;
        resp_framed_flip_read.nbytes = resp_framed.pos;
        resp_framed_flip_read.pos = 0;

        // Init for de-framing
        resp_deframed.bytes = resp->raw.data;
        resp_deframed.nbytes = sizeof(resp->raw);
        resp_deframed.pos = 0;

        hr = p3io_frame_decode(
            &resp_deframed, (struct const_iobuf *) &resp_framed_flip_read);

        iobuf_log(&resp_deframed, "p3iodrv-device response deframed");

        // Verify if the de-framed message is actually complete
        // +1 for the length byte
        if (resp->hdr.nbytes + 1 > resp_deframed.pos) {
            log_warning(
                "Truncated de-framed message, length %ld less than size on "
                "header %ld, read again",
                (DWORD) resp_deframed.pos,
                (DWORD) resp->hdr.nbytes);
            continue;
        }

        // Eror check decoding only after verification that frame is completed
        // Otherwise, this leads to failed decodings on truncated pakets that
        // require another read
        if (FAILED(hr)) {
            log_warning("Decoding response payload failed: %lX", hr);
            return hr;
        }

        log_misc(
            "Recieved complete response, length de-framed %ld",
            (DWORD) resp_deframed.pos);

        break;
    }

    return S_OK;
}

HRESULT p3iodrv_device_scan(char path[MAX_PATH])
{
    HRESULT res;
    PSP_DEVICE_INTERFACE_DETAIL_DATA_A detail_data;
    HDEVINFO devinfo;
    DWORD required_size;
    SP_DEVICE_INTERFACE_DATA interface_data;
    DWORD err;

    detail_data = NULL;

    devinfo = SetupDiGetClassDevsA(
        &p3io_guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (devinfo == INVALID_HANDLE_VALUE) {
        // No device with GUID found
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if (!SetupDiEnumDeviceInterfaces(
            devinfo, NULL, &p3io_guid, 0, &interface_data)) {
        res = HRESULT_FROM_WIN32(GetLastError());
        log_warning("SetupDiEnumDeviceInterfaces failed: %lX", res);
        SetupDiDestroyDeviceInfoList(devinfo);
        return res;
    }

    if (!SetupDiGetDeviceInterfaceDetailA(
            devinfo, &interface_data, NULL, 0, &required_size, NULL)) {
        err = GetLastError();
        res = HRESULT_FROM_WIN32(err);

        if (err != ERROR_INSUFFICIENT_BUFFER) {
            log_warning("SetupDiGetDeviceInterfaceDetailA failed: %lX", res);
            SetupDiDestroyDeviceInfoList(devinfo);
            return res;
        }
    }

    detail_data = malloc(required_size);
    detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

    if (!SetupDiGetDeviceInterfaceDetailA(
            devinfo, &interface_data, detail_data, required_size, NULL, NULL)) {
        res = HRESULT_FROM_WIN32(GetLastError());
        log_warning("SetupDiGetDeviceInterfaceDetailA failed: %lX", res);
        free(detail_data);
        SetupDiDestroyDeviceInfoList(devinfo);
        return res;
    }

    str_cpy(path, MAX_PATH, detail_data->DevicePath);
    str_cat(path, MAX_PATH, P3IO_DEVICE_FILENMAME);

    free(detail_data);
    SetupDiDestroyDeviceInfoList(devinfo);

    return S_OK;
}

HRESULT p3iodrv_device_open(const char *path, HANDLE *handle)
{
    HRESULT res;

    log_assert(path);
    log_assert(handle);

    *handle = CreateFileA(
        path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (*handle == INVALID_HANDLE_VALUE) {
        res = HRESULT_FROM_WIN32(GetLastError());
        log_warning("CreateFileA failed: %lX", res);
        return res;
    }

    return S_OK;
}

HRESULT p3iodrv_device_close(HANDLE *handle)
{
    HRESULT res;

    log_assert(handle);
    log_assert(*handle != INVALID_HANDLE_VALUE);

    if (CloseHandle(*handle) == FALSE) {
        res = HRESULT_FROM_WIN32(GetLastError());
        log_warning("Closing failed: %lX", res);
        return res;
    }

    *handle = INVALID_HANDLE_VALUE;

    return S_OK;
}

HRESULT p3iodrv_device_read_version(
    HANDLE handle, char version[P3IODRV_VERSION_MAX_LEN])
{
    HRESULT res;
    DWORD bytes_returned;

    log_assert(handle != INVALID_HANDLE_VALUE);

    memset(version, 0, sizeof(char[P3IODRV_VERSION_MAX_LEN]));

    if (!DeviceIoControl(
            handle,
            P3IO_IOCTL_GET_VERSION,
            NULL,
            0,
            version,
            sizeof(char[P3IODRV_VERSION_MAX_LEN]),
            &bytes_returned,
            NULL)) {
        res = HRESULT_FROM_WIN32(GetLastError());
        log_warning("DeviceIoControl failed: %lX", res);
        return res;
    }

    if (bytes_returned < 1) {
        log_warning(
            "DeviceIoControl returned size invalid: %ld", bytes_returned);
        return HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
    }

    return S_OK;
}

HRESULT
p3iodrv_device_read_jamma(HANDLE handle, uint32_t jamma[P3IO_DRV_JAMMA_MAX_LEN])
{
    HRESULT res;
    DWORD bytes_returned;

    log_assert(handle != INVALID_HANDLE_VALUE);

    if (!DeviceIoControl(
            handle,
            P3IO_IOCTL_READ_JAMMA,
            NULL,
            0,
            jamma,
            sizeof(uint32_t[P3IO_DRV_JAMMA_MAX_LEN]),
            &bytes_returned,
            NULL)) {
        res = HRESULT_FROM_WIN32(GetLastError());
        log_warning("ioctl read jamma failed: %lX", res);
        return res;
    }

    if (bytes_returned != sizeof(uint32_t[P3IO_DRV_JAMMA_MAX_LEN])) {
        log_warning(
            "DeviceIoControl returned size invalid: %ld", bytes_returned);
        return HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
    }

    return S_OK;
}

HRESULT p3iodrv_device_transfer(
    HANDLE handle, const union p3io_req_any *req, union p3io_resp_any *resp)
{
    HRESULT hr;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(req);
    log_assert(resp);

    log_misc(
        "TRANSFER START, req nbytes %d, seq_no %d, cmd 0x%X",
        req->hdr.nbytes,
        req->hdr.seq_no,
        req->hdr.cmd);

    hr = _p3iodrv_write_message(handle, req);

    if (FAILED(hr)) {
        return hr;
    }

    hr = _p3iodrv_read_full_response_message(handle, resp);

    if (FAILED(hr)) {
        return hr;
    }

    log_misc(
        "TRANSFER FINISHED, resp nbytes %d, seq_no %d, cmd 0x%X",
        resp->hdr.nbytes,
        resp->hdr.seq_no,
        resp->hdr.cmd);

    return S_OK;
}
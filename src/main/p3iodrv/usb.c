#define LOG_MODULE "p3iodrv-usb"

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
// clang-format on

#include "p3io/cmd.h"
#include "p3io/guid.h"
#include "p3io/ioctl.h"

#include "p3iodrv/usb.h"

#include "util/log.h"
#include "util/str.h"

HANDLE p3io_usb_open(void)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    wchar_t p3io_filename[MAX_PATH];
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail_data = NULL;
    HDEVINFO dev_info_set;

    dev_info_set = SetupDiGetClassDevsW(&p3io_guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if(dev_info_set == INVALID_HANDLE_VALUE) {
        log_warning("SetupDiGetClassDevs fail, is p4io device connected and driver installed?");
        return INVALID_HANDLE_VALUE;
    }

    SP_DEVICE_INTERFACE_DATA interface_data;
    interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if(!SetupDiEnumDeviceInterfaces(dev_info_set, NULL, &p3io_guid, 0, &interface_data)) {
        log_warning("SetupDiEnumDeviceInterfaces fail");
        goto CLEANUP;
    }

    DWORD required_size;
    SetupDiGetDeviceInterfaceDetailW(dev_info_set, &interface_data, NULL, 0, &required_size, NULL);

    detail_data = malloc(required_size);
    detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

    if(!SetupDiGetDeviceInterfaceDetailW(dev_info_set, &interface_data, detail_data, required_size, NULL, NULL)) {
        log_warning("SetupDiGetDeviceInterfaceDetailW fail");
        goto CLEANUP;
    }

    wstr_cpy(p3io_filename, MAX_PATH, detail_data->DevicePath);
    wstr_cat(p3io_filename, MAX_PATH, L"\\p3io");

    log_info("p3io found at path %ls", p3io_filename);

    handle = CreateFileW(p3io_filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(handle == INVALID_HANDLE_VALUE) {
        log_warning("CreateFileW fail");
        goto CLEANUP;
    }

    CLEANUP:
    free(detail_data);
    SetupDiDestroyDeviceInfoList(dev_info_set);

    return handle;
}

void p3io_usb_close(HANDLE p3io_handle)
{
    if (p3io_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(p3io_handle);
    }

    p3io_handle = INVALID_HANDLE_VALUE;
}

bool p3io_usb_read_jamma(HANDLE interrupt_handle, uint32_t jamma[3])
{
    DWORD bytes_returned;

    if(!DeviceIoControl(interrupt_handle, P3IO_IOCTL_READ_JAMMA, NULL, 0, jamma, sizeof(uint32_t[3]), &bytes_returned, NULL)) {
        log_warning("jamma read failed: %X", GetLastError());
        return false;
    }

    return true;
}

bool p3io_usb_read_version(HANDLE bulk_handle, char version[128])
{
    DWORD bytes_returned;

    if(!DeviceIoControl(bulk_handle, P3IO_IOCTL_GET_VERSION, NULL, 0, version, 128, &bytes_returned, NULL)) {
        log_warning("p3io does not support read_version cmd");
        return false;
    }

    return true;
}

/*
bool p3io_usb_transfer(
    HANDLE bulk_handle,
    uint8_t cmd,
    uint8_t seq_no,
    const void *req_payload,
    size_t req_payload_len,
    void *resp_payload,
    size_t *resp_payload_len
)
{
    DWORD bytes_processed;
    union p3io_req_any req_buf;
    union p3io_resp_any resp_buf;
    HRESULT res;

    if(req_payload_len > sizeof(req_buf)) {
        log_warning("request too big");
        return false;
    }

    res = p3io_frame_encode(&req_buf, req_payload, req_payload_len);

    if (res != S_OK) {
        log_warning("Encoding payload failed: %X", res);
        return false;
    }

    if(!WriteFile(bulk_handle, &req_buf, req_payload_len, &bytes_processed, 0)) {
        log_warning("WriteFile failed");
        return false;
    }

    if (bytes_processed != req_payload_len) {
        log_warning("WriteFile didn't finish");
        return false;
    }

    // must be 65 bytes or requests can stall - only 64 will ever be returned
    if(!ReadFile(bulk_handle, &resp_buf, 65, &bytes_processed, 0)) {
        log_warning("ReadFile (%u) failed", (unsigned)resp_payload_len);
        return false;
    }

    p3io_frame_decode(resp_payload, resp_buf)

    if(bytes_processed >= sizeof(struct p4io_cmd_header)) {
        memcpy(resp_payload, cmd_buf.payload, *resp_payload_len);
        *resp_payload_len = cmd_buf.header.payload_len;

        if(cmd_buf.header.sof != P4IO_SOF) {
            log_warning("Response bad header");
            return false;
        }

        if(seq_no != cmd_buf.header.seq_num) {
            log_warning("seq_num mismatch (ours %d !=  theirs %d)", seq_no, cmd_buf.header.seq_num);
            return false;
        }
    } else {
        *resp_payload_len = 0;
    }

    return true;
}
*/
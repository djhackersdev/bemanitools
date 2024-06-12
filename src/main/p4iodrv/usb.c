#define LOG_MODULE "p4iodrv-usb"

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
// clang-format on

#include "iface-core/log.h"

#include "p4io/cmd.h"
#include "p4io/guid.h"
#include "p4io/ioctl.h"

#include "p4iodrv/usb.h"

#include "util/str.h"

HANDLE p4io_usb_open(void)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    wchar_t p4io_filename[MAX_PATH]; // game uses 1024, but it shouldn't be that
                                     // long
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail_data = NULL;
    HDEVINFO dev_info_set;

    dev_info_set = SetupDiGetClassDevsW(
        &p4io_guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (dev_info_set == INVALID_HANDLE_VALUE) {
        log_warning(
            "SetupDiGetClassDevs fail, is p4io device connected and driver "
            "installed?");
        return INVALID_HANDLE_VALUE;
    }

    SP_DEVICE_INTERFACE_DATA interface_data;
    interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if (!SetupDiEnumDeviceInterfaces(
            dev_info_set, NULL, &p4io_guid, 0, &interface_data)) {
        log_warning("SetupDiEnumDeviceInterfaces fail");
        goto CLEANUP;
    }

    DWORD required_size;
    SetupDiGetDeviceInterfaceDetailW(
        dev_info_set, &interface_data, NULL, 0, &required_size, NULL);

    detail_data = malloc(required_size);
    detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

    if (!SetupDiGetDeviceInterfaceDetailW(
            dev_info_set,
            &interface_data,
            detail_data,
            required_size,
            NULL,
            NULL)) {
        log_warning("SetupDiGetDeviceInterfaceDetailW fail");
        goto CLEANUP;
    }

    wstr_cpy(p4io_filename, MAX_PATH, detail_data->DevicePath);
    wstr_cat(p4io_filename, MAX_PATH, L"\\p4io");

    log_info("p4io found at path %ls", p4io_filename);

    handle = CreateFileW(
        p4io_filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        log_warning("CreateFileW fail");
        goto CLEANUP;
    }

CLEANUP:
    free(detail_data);
    SetupDiDestroyDeviceInfoList(dev_info_set);

    return handle;
}
void p4io_usb_close(HANDLE p4io_handle)
{
    if (p4io_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(p4io_handle);
    }
    p4io_handle = INVALID_HANDLE_VALUE;
}

bool p4io_usb_read_jamma(HANDLE interrupt_handle, uint32_t jamma[4])
{
    DWORD bytes_returned;
    if (!DeviceIoControl(
            interrupt_handle,
            P4IO_IOCTL_READ_JAMMA_2,
            NULL,
            0,
            jamma,
            sizeof(uint32_t[4]),
            &bytes_returned,
            NULL)) {
        log_warning("jamma read failed");
        return false;
    }

    return true;
}

bool p4io_usb_read_device_name(HANDLE bulk_handle, char name[128])
{
    DWORD bytes_returned;
    if (!DeviceIoControl(
            bulk_handle,
            P4IO_IOCTL_GET_DEVICE_NAME,
            NULL,
            0,
            name,
            128,
            &bytes_returned,
            NULL)) {
        log_warning("p4io does not support get_name cmd");
        return false;
    }

    return true;
}

bool p4io_usb_transfer(
    HANDLE bulk_handle,
    uint8_t cmd,
    uint8_t seq_no,
    const void *req_payload,
    size_t req_payload_len,
    void *resp_payload,
    size_t *resp_payload_len)
{
    DWORD bytes_requested;
    DWORD bytes_xferred;
    struct p4io_cmd_package cmd_buf;

    if (bulk_handle == INVALID_HANDLE_VALUE) {
        log_warning("p4io not open");
        return false;
    }

    if (req_payload_len > sizeof(cmd_buf.payload)) {
        log_warning("request too big");
        return false;
    }

    cmd_buf.header.sof = P4IO_SOF;
    cmd_buf.header.cmd = cmd;
    cmd_buf.header.seq_num = seq_no;
    cmd_buf.header.payload_len = req_payload_len;
    memcpy(cmd_buf.payload, req_payload, req_payload_len);

    bytes_requested = P4IO_CMD_HEADER_LEN + req_payload_len;

    if (!WriteFile(bulk_handle, &cmd_buf, bytes_requested, &bytes_xferred, 0)) {
        log_warning("WriteFile failed");
        return false;
    }
    if (bytes_xferred != bytes_requested) {
        log_warning("WriteFile didn't finish");
        return false;
    }

    // must be 65 bytes or requests can stall - only 64 will ever be returned
    if (!ReadFile(bulk_handle, &cmd_buf, 65, &bytes_xferred, 0)) {
        log_warning("ReadFile (%u) failed", (unsigned) bytes_requested);
        return false;
    }

    if (bytes_xferred >= sizeof(struct p4io_cmd_header)) {
        memcpy(resp_payload, cmd_buf.payload, *resp_payload_len);
        *resp_payload_len = cmd_buf.header.payload_len;

        if (cmd_buf.header.sof != P4IO_SOF) {
            log_warning("Response bad header");
            return false;
        }

        if (seq_no != cmd_buf.header.seq_num) {
            log_warning(
                "seq_num mismatch (ours %d !=  theirs %d)",
                seq_no,
                cmd_buf.header.seq_num);
            return false;
        }
    } else {
        *resp_payload_len = 0;
    }

    return true;
}

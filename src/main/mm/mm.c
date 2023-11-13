#include <initguid.h>
#include <shlobj.h>
#include <windows.h>

#include <hidsdi.h>
#include <setupapi.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mm/mm.h"

#include "util/cmdline.h"
#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

DEFINE_GUID(
    hid_guid,
    0x4D1E55B2L,
    0xF16F,
    0x11CF,
    0x88,
    0xCB,
    0x00,
    0x11,
    0x11,
    0x00,
    0x00,
    0x30);

static HANDLE mm_fd;

static struct mm_input *mm_in_bufs;
static int mm_in_buf_count;
static int mm_in_buf_pos;
static DWORD mm_in_xferred;
static OVERLAPPED mm_in_ovl;

static struct mm_output mm_out_buf;
static OVERLAPPED mm_out_ovl;
static bool mm_out_pending;

static HANDLE mm_open_device(void);
static HANDLE
mm_try_device(HDEVINFO dev_info, SP_DEVICE_INTERFACE_DATA *iface_data);

static HANDLE mm_open_device(void)
{
    HDEVINFO dev_info;
    SP_DEVICE_INTERFACE_DATA iface_data;
    HANDLE cur_fd;
    DWORD i;
    BOOL ok;

    dev_info = SetupDiGetClassDevsW(
        &hid_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (dev_info == INVALID_HANDLE_VALUE) {
        log_fatal("SetupDiGetClassDevs failed");
    }

    for (i = 0, cur_fd = NULL; cur_fd == NULL; i++) {
        iface_data.cbSize = sizeof(iface_data);

        ok = SetupDiEnumDeviceInterfaces(
            dev_info, NULL, &hid_guid, i, &iface_data);

        if (!ok) {
            if (GetLastError() == ERROR_NO_MORE_ITEMS) {
                break;
            } else {
                log_fatal(
                    "SetupDiEnumDeviceInterfaces failed: %#x",
                    (int) GetLastError());
            }
        }

        cur_fd = mm_try_device(dev_info, &iface_data);
    }

    SetupDiDestroyDeviceInfoList(dev_info);

    return cur_fd;
}

static HANDLE
mm_try_device(HDEVINFO dev_info, SP_DEVICE_INTERFACE_DATA *iface_data)
{
    SP_DEVICE_INTERFACE_DETAIL_DATA_W *detail;
    HIDD_ATTRIBUTES hid_attrs;
    HANDLE cur_fd;
    DWORD detail_size;
    BOOL ok;

    ok = SetupDiGetDeviceInterfaceDetailW(
        dev_info, iface_data, NULL, 0, &detail_size, NULL);

    if (!ok && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        log_fatal(
            "SetupDiGetDeviceInterfaceDetail sizing failed: %#x",
            (int) GetLastError());
    }

    detail = xmalloc(detail_size);
    detail->cbSize = sizeof(*detail);

    ok = SetupDiGetDeviceInterfaceDetailW(
        dev_info, iface_data, detail, detail_size, NULL, NULL);

    if (!ok) {
        log_fatal(
            "SetupDiGetDeviceInterfaceDetail failed: %#x",
            (int) GetLastError());
    }

    cur_fd = CreateFileW(
        detail->DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    free(detail);

    if (cur_fd == INVALID_HANDLE_VALUE) {
        goto open_fail;
    }

    ok = HidD_GetAttributes(cur_fd, &hid_attrs);

    if (!ok) {
        log_fatal("HidD_GetAttributes failed");
    }

    if (hid_attrs.VendorID != MM_VENDOR_ID ||
        hid_attrs.ProductID != MM_PRODUCT_ID) {
        goto id_fail;
    }

    return cur_fd;

id_fail:
    CloseHandle(cur_fd);

open_fail:
    return NULL;
}

bool mm_init(uint32_t in_buf_count)
{
    mm_in_buf_pos = 0;
    mm_in_buf_count = in_buf_count;
    mm_in_bufs = xcalloc(sizeof(*mm_in_bufs) * in_buf_count);

    log_info("Searching for MiniMaid ...");

    mm_fd = mm_open_device();

    if (mm_fd == NULL) {
        log_warning("MiniMaid not found");

        return false;
    }

    if (!HidD_SetNumInputBuffers(mm_fd, mm_in_buf_count)) {
        log_fatal("HidD_SetNumInputBuffers failed");
    }

    log_info("MiniMaid connected");

    memset(&mm_in_ovl, 0, sizeof(mm_in_ovl));
    mm_in_ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    memset(&mm_out_ovl, 0, sizeof(mm_out_ovl));
    mm_out_ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    /* Start up buffer chain */

    mm_in_xferred = sizeof(struct mm_input);

    do {
        mm_in_buf_pos = (mm_in_buf_pos + 1) % mm_in_buf_count;
    } while (ReadFile(
        mm_fd,
        &mm_in_bufs[mm_in_buf_pos],
        sizeof(*mm_in_bufs),
        NULL,
        &mm_in_ovl));

    if (GetLastError() != ERROR_IO_PENDING) {
        log_fatal("USB IN failed: %#x", (int) GetLastError());
    }

    return true;
}

void mm_update(const struct mm_output *out, struct mm_input *in)
{
    DWORD xferred;

    /* Lights go first; we defer the input read as much as possible in order
       to return the freshest possible pad state */

    if (mm_out_pending) {
        /* Only try to send lights once. If the OUT transfer is still in
           flight then we'll retire the IO on the next poll cycle. */

        if (GetOverlappedResult(mm_fd, &mm_out_ovl, &xferred, FALSE)) {
            ResetEvent(mm_out_ovl.hEvent);

            mm_out_pending = false;
        } else {
            if (GetLastError() != ERROR_IO_INCOMPLETE) {
                log_fatal(
                    "GetOverlappedResult[OUT] failed: %#x",
                    (int) GetLastError());
            }
        }
    }

    /* Kick off an async write if one isn't already in progress. We try to
       maintain a continuous transfer of OUT reports (even if no lights have
       changed) in order to keep the bus utilisation consistent and hence
       minimise timing jitter. */

    if (!mm_out_pending) {
        memcpy(&mm_out_buf, out, sizeof(*out));

        if (!WriteFile(
                mm_fd, &mm_out_buf, sizeof(mm_out_buf), NULL, &mm_out_ovl)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                log_fatal("USB OUT failed: %#x", (int) GetLastError());
            }
        }

        mm_out_pending = true;
    }

    /* Last TRUE here means that we block until an IN report is available. */

    if (!GetOverlappedResult(mm_fd, &mm_in_ovl, &mm_in_xferred, TRUE)) {
        log_fatal("GetOverlappedResult[IN] failed: %#x", (int) GetLastError());
    }

    /* Pump whatever backlog of reports is available until we block */

    do {
        mm_in_buf_pos = (mm_in_buf_pos + 1) % mm_in_buf_count;
    } while (ReadFile(
        mm_fd,
        &mm_in_bufs[mm_in_buf_pos],
        sizeof(*mm_in_bufs),
        NULL,
        &mm_in_ovl));

    /* (did we actually block or was there an IO error?) */

    if (GetLastError() != ERROR_IO_PENDING) {
        log_fatal("USB IN failed: %#x", (int) GetLastError());
    }

    memcpy(in, &mm_in_bufs[(mm_in_buf_pos + 1) % mm_in_buf_count], sizeof(*in));
}

void mm_fini(void)
{
    CloseHandle(mm_fd);
    CloseHandle(mm_out_ovl.hEvent);
    CloseHandle(mm_in_ovl.hEvent);

    log_info("Closed MiniMaid");
}

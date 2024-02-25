// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <dbt.h>
// clang-format on

#include <string.h>

#include "core/log.h"

#include "geninput/hid.h"
#include "geninput/hotplug.h"
#include "geninput/io-thread.h"
#include "geninput/ri.h"

static HDEVNOTIFY hotplug_handle;

void hotplug_init(HWND wnd)
{
    DEV_BROADCAST_DEVICEINTERFACE filter;

    memset(&filter, 0, sizeof(filter));
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = hid_guid;

    hotplug_handle =
        RegisterDeviceNotification(wnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

    if (hotplug_handle == NULL) {
        log_fatal(
            "Failed to register for HID hotplug notifications: %08x",
            (unsigned int) GetLastError());
    }
}

void hotplug_handle_msg(WPARAM wparam, const DEV_BROADCAST_HDR *hdr)
{
    const DEV_BROADCAST_DEVICEINTERFACE *dev;

    if (wparam != DBT_DEVICEARRIVAL) {
        return;
    }

    if (hdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE) {
        return;
    }

    dev = (const DEV_BROADCAST_DEVICEINTERFACE *) hdr;

    if (memcmp(&dev->dbcc_classguid, &hid_guid, sizeof(hid_guid))) {
        return;
    }

    log_misc("HID hotplug detected: %s", dev->dbcc_name);
    io_thread_add_device(dev->dbcc_name);
    ri_scan_devices();
}

void hotplug_fini(void)
{
    BOOL ok;

    ok = UnregisterDeviceNotification(hotplug_handle);

    if (!ok) {
        log_warning(
            "Failed to unregister HID hotplug notifications: %08x",
            (unsigned int) GetLastError());
    }
}

#include <stdlib.h>

#include "geninput/hid-mgr.h"
#include "geninput/hid.h"
#include "geninput/kbd.h"
#include "geninput/mouse.h"
#include "geninput/ri.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/mem.h"

struct ri_handle {
    HANDLE fake_fd;
    struct hid_ri *hid_ri;
};

static struct ri_handle *ri_handles;
static unsigned int ri_ndevs;

void ri_init(HWND hwnd)
{
    RAWINPUTDEVICE filter[3];

    memset(filter, 0, sizeof(filter));

    filter[0].usUsagePage = KBD_DEVICE_USAGE_KEYBOARD >> 16;
    filter[0].usUsage = (uint16_t) KBD_DEVICE_USAGE_KEYBOARD;
    filter[0].dwFlags = RIDEV_INPUTSINK;
    filter[0].hwndTarget = hwnd;

    filter[1].usUsagePage = KBD_DEVICE_USAGE_KEYPAD >> 16;
    filter[1].usUsage = (uint16_t) KBD_DEVICE_USAGE_KEYPAD;
    filter[1].dwFlags = RIDEV_INPUTSINK;
    filter[1].hwndTarget = hwnd;

    filter[2].usUsagePage = MOUSE_DEVICE_USAGE >> 16;
    filter[2].usUsage = (uint16_t) MOUSE_DEVICE_USAGE;
    filter[2].dwFlags = RIDEV_INPUTSINK;
    filter[2].hwndTarget = hwnd;

    if (!RegisterRawInputDevices(filter, lengthof(filter), sizeof(filter[0]))) {
        log_fatal(
            "RegisterRawInputDevices failed: %x",
            (unsigned int) GetLastError());
    }

    ri_scan_devices();
}

void ri_scan_devices(void)
{
    char *dev_node;
    struct hid_stub *stub;
    RAWINPUTDEVICELIST *ridl;
    unsigned int ridl_size;
    unsigned int size;
    unsigned int i;

    hid_mgr_lock();

    GetRawInputDeviceList(NULL, &ri_ndevs, sizeof(*ridl));

    /* Param 2 of GetRawInputDeviceList changes meaning depending on whether or
       not param 1 is NULL */

    ridl_size = sizeof(*ridl) * ri_ndevs;
    ridl = xmalloc(ridl_size);

    if (GetRawInputDeviceList(ridl, &ridl_size, sizeof(*ridl)) == -1) {
        log_fatal("GetRawInputDeviceList data failed");
    }

    ri_handles = xrealloc(ri_handles, sizeof(*ri_handles) * ri_ndevs);

    for (i = 0; i < ri_ndevs; i++) {
        ri_handles[i].fake_fd = NULL;
        ri_handles[i].hid_ri = NULL;

        size = 0;
        GetRawInputDeviceInfo(ridl[i].hDevice, RIDI_DEVICENAME, NULL, &size);

        dev_node = xmalloc(size);

        if (GetRawInputDeviceInfo(
                ridl[i].hDevice, RIDI_DEVICENAME, dev_node, &size) == -1) {
            log_warning("GetRawInputDeviceInfo(RIDI_DEVICENAME) data failed");

            goto skip;
        }

        if (size > 1 && dev_node[0] == '\\' && dev_node[1] == '?') {
            /* WinXP (and only WinXP, not Vista or up) return device nodes
               starting with \??\ here instead of \\?\ like every other part
               of Windows does. I'm assuming this is a bug. */
            dev_node[1] = '\\';
        }

        stub = hid_mgr_get_named_stub(dev_node);

        /* NOTE: hid_mgr owns the ridev, not us. Raw Input devices do not
           get destroyed until geninput shuts down, they merely cease to
           update if the corresponding physical device is unplugged. */

        if (ridl[i].dwType == RIM_TYPEKEYBOARD) {
            ri_handles[i].fake_fd = ridl[i].hDevice;
            kbd_create(&ri_handles[i].hid_ri, dev_node);

            hid_stub_attach(stub, (struct hid *) ri_handles[i].hid_ri);
        } else if (ridl[i].dwType == RIM_TYPEMOUSE) {
            ri_handles[i].fake_fd = ridl[i].hDevice;
            mouse_create(&ri_handles[i].hid_ri, dev_node);

            hid_stub_attach(stub, (struct hid *) ri_handles[i].hid_ri);
        }

    skip:
        free(dev_node);
    }

    free(ridl);

    hid_mgr_unlock();
}

void ri_handle_msg(HRAWINPUT msg)
{
    unsigned int i;
    unsigned int nbytes;
    RAWINPUT ri;

    nbytes = sizeof(ri);

    if (GetRawInputData(msg, RID_INPUT, &ri, &nbytes, sizeof(ri.header)) ==
        (UINT) -1) {
        log_warning(
            "GetRawInputData failed: %x", (unsigned int) GetLastError());

        return;
    }

    if (ri.header.hDevice == NULL) {
        /* WTF?? I've seen this happen while remote-controlling someone's
           desktop using TeamViewer, possibly due to an API hook injected by TV
           doing something strange. However, if it's actually me misusing the
           API somehow then I'd like to trap this case so that it doesn't blow
           up my RawInput dispatcher. */
        return;
    }

    for (i = 0; i < ri_ndevs; i++) {
        if (ri_handles[i].fake_fd == ri.header.hDevice) {
            hid_ri_handle_event(ri_handles[i].hid_ri, &ri);

            return;
        }
    }
}

void ri_fini(void)
{
    RAWINPUTDEVICE filter[3];

    free(ri_handles);
    ri_handles = NULL;
    ri_ndevs = 0;

    filter[0].usUsagePage = KBD_DEVICE_USAGE_KEYBOARD >> 16;
    filter[0].usUsage = (uint16_t) KBD_DEVICE_USAGE_KEYBOARD;
    filter[0].dwFlags = RIDEV_REMOVE;
    filter[0].hwndTarget = NULL;

    filter[1].usUsagePage = KBD_DEVICE_USAGE_KEYPAD >> 16;
    filter[1].usUsage = (uint16_t) KBD_DEVICE_USAGE_KEYPAD;
    filter[1].dwFlags = RIDEV_REMOVE;
    filter[1].hwndTarget = NULL;

    filter[2].usUsagePage = MOUSE_DEVICE_USAGE >> 16;
    filter[2].usUsage = (uint16_t) MOUSE_DEVICE_USAGE;
    filter[2].dwFlags = RIDEV_REMOVE;
    filter[2].hwndTarget = NULL;

    if (!RegisterRawInputDevices(filter, lengthof(filter), sizeof(filter[0]))) {
        log_warning(
            "RegisterRawInputDevices(RIDEV_REMOVE) failed: %x",
            (unsigned int) GetLastError());
    }
}

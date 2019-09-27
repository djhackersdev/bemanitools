#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>

#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "geninput/dev-list.h"

#include "util/log.h"
#include "util/mem.h"

void dev_list_init(struct dev_list *devs, const GUID *class_guid)
{
    devs->class_guid = (GUID *) class_guid;
    devs->infolist = SetupDiGetClassDevs(devs->class_guid, NULL, NULL,
               DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (devs->infolist == NULL) {
        log_fatal("SetupDiGetClassDevs failed: %08x",
                (unsigned int) GetLastError());
    }

    devs->dev_no = 0;
    devs->iface_no = 0;

    devs->detail = NULL;
    devs->name = NULL;
}

bool dev_list_next(struct dev_list *devs)
{
    if (devs->detail != NULL) {
        free(devs->detail);

        devs->detail = NULL;
    }

    if (devs->name != NULL) {
        free(devs->name);

        devs->name = NULL;
    }

    for (;;) {
        devs->dev.cbSize = sizeof(devs->dev);

        if (!SetupDiEnumDeviceInfo(devs->infolist, devs->dev_no, &devs->dev)) {
            if (GetLastError() == ERROR_NO_MORE_ITEMS) {
                return false;
            } else {
                log_fatal("SetupDiEnumDeviceInfo failed: %08x",
                        (unsigned int) GetLastError());
            }
        }

        devs->iface.cbSize = sizeof(devs->iface);

        if (!SetupDiEnumDeviceInterfaces(devs->infolist, &devs->dev,
                devs->class_guid, devs->iface_no, &devs->iface)) {
            if (GetLastError() == ERROR_NO_MORE_ITEMS) {
                devs->dev_no++;
                devs->iface_no = 0;

                continue;
            } else {
                log_fatal("SetupDiEnumDeviceInterfaces failed: %08x",
                        (unsigned int) GetLastError());
            }
        }

        devs->iface_no++;

        return true;
    }
}

const char *dev_list_get_dev_node(struct dev_list *devs)
{
    DWORD detail_size;

    if (devs->detail != NULL) {
        return devs->detail->DevicePath;
    }

    if (SetupDiGetDeviceInterfaceDetail(devs->infolist, &devs->iface, NULL, 0,
            &detail_size, NULL)
            || GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        log_fatal("SetupDiGetDeviceInterfaceDetail sizing failed: %08x",
                (unsigned int) GetLastError());
    }

    devs->detail = xmalloc(detail_size);
    devs->detail->cbSize = sizeof(*devs->detail);

    if (!SetupDiGetDeviceInterfaceDetail(devs->infolist, &devs->iface,
            devs->detail, detail_size, NULL, NULL)) {
         log_fatal("SetupDiGetDeviceInterfaceDetail content failed: %08x",
                (unsigned int) GetLastError());
    }

    return devs->detail->DevicePath;
}

const wchar_t *dev_list_get_dev_name(struct dev_list *devs)
{
    DWORD name_size;
    const char* dev_path;
    // largest possible product string according to WinAPI docs
    wchar_t product_string[126];
    DWORD product_size;
    bool product_success;

    if (devs->name != NULL) {
        return devs->name;
    }

    if (SetupDiGetDeviceRegistryPropertyW(devs->infolist, &devs->dev,
            SPDRP_DEVICEDESC, NULL, NULL, 0, &name_size)
            || GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        log_fatal("SetupDiGetDeviceRegistryPropertyW(SPDRP_DEVICEDESC) sizing "
                "failed: %08x", (unsigned int) GetLastError());
    }

    /* This function returns arbitrary data, not just strings. Therefore, we
       are actually working in units of bytes and not wchar_t's here for once */

    devs->name = xmalloc(name_size);

    if (!SetupDiGetDeviceRegistryPropertyW(devs->infolist, &devs->dev,
            SPDRP_DEVICEDESC, NULL, (void *) devs->name, name_size, 0)) {
        log_fatal("SetupDiGetDeviceRegistryPropertyW(SPDRP_DEVICEDESC) content "
                "failed: %08x", (unsigned int) GetLastError());
    }

    /* Also, try and get the product string - this is usually far more human
       readable than the registry value, which is often "HID Compliant Thing".
       We then append this in parentheses */
    dev_path = dev_list_get_dev_node(devs);
    HANDLE hid_hnd = CreateFile(dev_path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(hid_hnd) {
        product_success = HidD_GetProductString(hid_hnd, product_string, sizeof(product_string));
        CloseHandle(hid_hnd);
        if(product_success && wcslen(product_string) > 0 && wcscmp(product_string, devs->name)) {
            // len + null + 2*parentheses + space separator
            product_size = (wcslen(product_string)+4) * sizeof(wchar_t);
            devs->name = xrealloc(devs->name, name_size + product_size);
            wcscat(devs->name, L" (");
            wcscat(devs->name, product_string);
            wcscat(devs->name, L")");
        }
    }

    return devs->name;
}

void dev_list_fini(struct dev_list *devs)
{
    if (devs->detail != NULL) {
        free(devs->detail);
    }

    if (devs->name != NULL) {
        free(devs->name);
    }

    if (!SetupDiDestroyDeviceInfoList(devs->infolist)) {
        log_fatal("SetupDiDestroyDeviceList failed: %08x",
                (unsigned int) GetLastError());
    }
}


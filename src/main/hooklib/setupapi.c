#define LOG_MODULE "hook-setupapi"

#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "hook/table.h"

#include "hooklib/setupapi.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

/* my hooks */

static BOOL STDCALL my_SetupDiDestroyDeviceInfoList(HDEVINFO dev_info);

static BOOL STDCALL my_SetupDiEnumDeviceInfo(
    HDEVINFO dev_info, DWORD index, SP_DEVINFO_DATA *info_data);

static BOOL STDCALL my_SetupDiEnumDeviceInterfaces(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    const GUID *iface_guid,
    DWORD index,
    SP_DEVICE_INTERFACE_DATA *ifd);

static HDEVINFO STDCALL my_SetupDiGetClassDevsA(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags);

static HDEVINFO STDCALL my_SetupDiGetClassDevsW(
    const GUID *class_guid, PCWSTR enumerator, HWND hwnd, DWORD flags);

static BOOL STDCALL my_SetupDiGetDeviceInterfaceDetailA(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_A *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data);

static BOOL STDCALL my_SetupDiGetDeviceInterfaceDetailW(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_W *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data);

static BOOL STDCALL my_SetupDiGetDeviceRegistryPropertyA(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    DWORD prop_id,
    DWORD *prop_type,
    void *bytes,
    DWORD nbytes,
    DWORD *required_nbytes);

/* real calls */

static BOOL(STDCALL *real_SetupDiDestroyDeviceInfoList)(HDEVINFO dev_info);

static BOOL(STDCALL *real_SetupDiEnumDeviceInfo)(
    HDEVINFO dev_info, DWORD index, SP_DEVINFO_DATA *info_data);

static BOOL(STDCALL *real_SetupDiEnumDeviceInterfaces)(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    const GUID *iface_guid,
    DWORD index,
    SP_DEVICE_INTERFACE_DATA *ifd);

static HDEVINFO(STDCALL *real_SetupDiGetClassDevsA)(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags);

static HDEVINFO(STDCALL *real_SetupDiGetClassDevsW)(
    const GUID *class_guid, PCWSTR enumerator, HWND hwnd, DWORD flags);

static BOOL(STDCALL *real_SetupDiGetDeviceInterfaceDetailA)(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_A *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data);

static BOOL(STDCALL *real_SetupDiGetDeviceInterfaceDetailW)(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_W *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data);

static BOOL(STDCALL *real_SetupDiGetDeviceRegistryPropertyA)(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    DWORD prop_id,
    DWORD *prop_type,
    void *bytes,
    DWORD nbytes,
    DWORD *required_nbytes);

static const struct hook_symbol setupapi_hook_syms[] = {
    {.name = "SetupDiDestroyDeviceInfoList",
     .patch = my_SetupDiDestroyDeviceInfoList,
     .link = (void **) &real_SetupDiDestroyDeviceInfoList},
    {.name = "SetupDiEnumDeviceInfo",
     .patch = my_SetupDiEnumDeviceInfo,
     .link = (void **) &real_SetupDiEnumDeviceInfo},
    {.name = "SetupDiEnumDeviceInterfaces",
     .patch = my_SetupDiEnumDeviceInterfaces,
     .link = (void **) &real_SetupDiEnumDeviceInterfaces},
    {.name = "SetupDiGetClassDevsA",
     .patch = my_SetupDiGetClassDevsA,
     .link = (void **) &real_SetupDiGetClassDevsA},
    {.name = "SetupDiGetClassDevsW",
     .patch = my_SetupDiGetClassDevsW,
     .link = (void **) &real_SetupDiGetClassDevsW},
    {.name = "SetupDiGetDeviceInterfaceDetailA",
     .patch = my_SetupDiGetDeviceInterfaceDetailA,
     .link = (void **) &real_SetupDiGetDeviceInterfaceDetailA},
    {.name = "SetupDiGetDeviceInterfaceDetailW",
     .patch = my_SetupDiGetDeviceInterfaceDetailW,
     .link = (void **) &real_SetupDiGetDeviceInterfaceDetailW},
    {.name = "SetupDiGetDeviceRegistryPropertyA",
     .patch = my_SetupDiGetDeviceRegistryPropertyA,
     .link = (void **) &real_SetupDiGetDeviceRegistryPropertyA},
};

static int hook_setupapi_fake_handle;
static const struct hook_setupapi_data *hook_setupapi_data;

static HDEVINFO STDCALL my_SetupDiGetClassDevsA(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags)
{
    /* That's how iidx 9-13 detected the old C02 IO. That doesn't work
       on iidx 14 anymore... */
    /*
    if (class_guid != NULL
            && memcmp(class_guid, &hook_setupapi_data->device_guid,
            sizeof(hook_setupapi_data->device_guid)) == 0) {
    */
    if ((class_guid != NULL &&
         !memcmp(
             class_guid,
             &hook_setupapi_data->device_guid,
             sizeof(hook_setupapi_data->device_guid))) ||
        (enumerator != NULL && !strcmp(enumerator, "USB"))) {
        SetLastError(ERROR_SUCCESS);

        log_misc("SetupDiGetClassDevsA: %s", hook_setupapi_data->device_path);

        return &hook_setupapi_fake_handle;
    } else {
        return real_SetupDiGetClassDevsA(class_guid, enumerator, hwnd, flags);
    }
}

static HDEVINFO STDCALL my_SetupDiGetClassDevsW(
    const GUID *class_guid, PCWSTR enumerator, HWND hwnd, DWORD flags)
{
    if (class_guid != NULL &&
        memcmp(
            class_guid,
            &hook_setupapi_data->device_guid,
            sizeof(hook_setupapi_data->device_guid)) == 0) {
        SetLastError(ERROR_SUCCESS);

        log_misc("SetupDiGetClassDevsW: %s", hook_setupapi_data->device_path);

        return &hook_setupapi_fake_handle;
    } else {
        return real_SetupDiGetClassDevsW(class_guid, enumerator, hwnd, flags);
    }
}

static BOOL STDCALL my_SetupDiEnumDeviceInfo(
    HDEVINFO dev_info, DWORD index, SP_DEVINFO_DATA *info_data)
{
    if (dev_info == &hook_setupapi_fake_handle) {
        if (index == 0) {
            SetLastError(ERROR_SUCCESS);

            log_misc("SetupDiEnumDeviceInfo");

            return TRUE;
        } else {
            SetLastError(ERROR_NO_MORE_ITEMS);

            return FALSE;
        }
    } else {
        return real_SetupDiEnumDeviceInfo(dev_info, index, info_data);
    }
}

static BOOL STDCALL my_SetupDiEnumDeviceInterfaces(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    const GUID *iface_guid,
    DWORD index,
    SP_DEVICE_INTERFACE_DATA *ifd)
{
    if (dev_info == &hook_setupapi_fake_handle) {
        SetLastError(ERROR_SUCCESS);

        log_misc("SetupDiEnumDeviceInterfaces");

        return index == 0;
    } else {
        return real_SetupDiEnumDeviceInterfaces(
            dev_info, info_data, iface_guid, index, ifd);
    }
}

static BOOL STDCALL my_SetupDiGetDeviceRegistryPropertyA(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    DWORD prop_id,
    DWORD *prop_type,
    void *bytes,
    DWORD nbytes,
    DWORD *required_nbytes)
{
    if (dev_info == &hook_setupapi_fake_handle) {
        if (prop_id != SPDRP_DEVICEDESC) {
            SetLastError(ERROR_BAD_ARGUMENTS);

            return FALSE;
        }

        if (nbytes < MAX_PATH) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            *required_nbytes = MAX_PATH;

            return FALSE;
        } else {
            SetLastError(ERROR_SUCCESS);

            *prop_type = REG_SZ;

            if (hook_setupapi_data->device_desc) {
                log_misc(
                    "SetupDiGetDeviceRegistryPropertyA: %s",
                    hook_setupapi_data->device_desc);
                str_cpy(bytes, nbytes, hook_setupapi_data->device_desc);
            }

            return TRUE;
        }

    } else {
        return real_SetupDiGetDeviceRegistryPropertyA(
            dev_info,
            info_data,
            prop_id,
            prop_type,
            bytes,
            nbytes,
            required_nbytes);
    }
}

static BOOL STDCALL my_SetupDiGetDeviceInterfaceDetailA(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_A *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data)
{
    if (dev_info == &hook_setupapi_fake_handle) {
        if (size == 0) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            *required_size = sizeof(DWORD) + sizeof(char) * MAX_PATH;

            return FALSE;
        } else {
            SetLastError(ERROR_SUCCESS);

            log_misc(
                "SetupDiGetDeviceInterfaceDetailA: %s",
                hook_setupapi_data->device_path);

            detail->cbSize = strlen(hook_setupapi_data->device_path);
            memcpy(
                detail->DevicePath,
                hook_setupapi_data->device_path,
                detail->cbSize);
            detail->DevicePath[detail->cbSize] = '\0';

            return TRUE;
        }
    } else {
        return real_SetupDiGetDeviceInterfaceDetailA(
            dev_info, ifd, detail, size, required_size, info_data);
    }
}

static BOOL STDCALL my_SetupDiGetDeviceInterfaceDetailW(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_W *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data)
{
    if (dev_info == &hook_setupapi_fake_handle) {
        if (size == 0) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            *required_size = sizeof(DWORD) + sizeof(wchar_t) * MAX_PATH;

            return FALSE;
        } else {
            SetLastError(ERROR_SUCCESS);

            log_misc(
                "SetupDiGetDeviceInterfaceDetailW: %s",
                hook_setupapi_data->device_path);

            wchar_t *wstr_path = str_widen(hook_setupapi_data->device_path);

            detail->cbSize = strlen(hook_setupapi_data->device_path);
            memcpy(
                detail->DevicePath,
                wstr_path,
                (wcslen(wstr_path) + 1) * sizeof(wchar_t));
            detail->DevicePath[detail->cbSize] = '\0';

            free(wstr_path);

            return TRUE;
        }

    } else {
        return real_SetupDiGetDeviceInterfaceDetailW(
            dev_info, ifd, detail, size, required_size, info_data);
    }
}

static BOOL STDCALL my_SetupDiDestroyDeviceInfoList(HDEVINFO dev_info)
{
    if (dev_info == &hook_setupapi_fake_handle) {
        log_misc("SetupDiDestroyDeviceInfoList");

        return TRUE;
    } else {
        return real_SetupDiDestroyDeviceInfoList(dev_info);
    }
}

void hook_setupapi_init(const struct hook_setupapi_data *data)
{
    hook_table_apply(
        NULL, "setupapi.dll", setupapi_hook_syms, lengthof(setupapi_hook_syms));

    hook_setupapi_data = data;
    log_info(
        "Hooked setupapi for %s, %s", data->device_path, data->device_desc);
}

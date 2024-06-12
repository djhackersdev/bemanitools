// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
// clang-format on

#include <stdbool.h>
#include <string.h>

#include "hook/table.h"

#include "iface-core/log.h"

#include "p3io/guid.h"

#include "p3ioemu/devmgr.h"

#include "util/defs.h"
#include "util/str.h"

/* Link pointers */

static HDEVINFO(WINAPI *next_SetupDiGetClassDevsW)(
    const GUID *class_guid, const wchar_t *enumerator, HWND hwnd, DWORD flags);

static HDEVINFO(WINAPI *next_SetupDiGetClassDevsA)(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags);

static BOOL(WINAPI *next_SetupDiEnumDeviceInterfaces)(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    const GUID *iface_guid,
    DWORD index,
    SP_DEVICE_INTERFACE_DATA *ifd);

static BOOL(WINAPI *next_SetupDiGetDeviceInterfaceDetailW)(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_W *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data);

static BOOL(WINAPI *next_SetupDiDestroyDeviceInfoList)(HDEVINFO dev_info);

/* API hooks */

static HDEVINFO WINAPI my_SetupDiGetClassDevsW(
    const GUID *class_guid, const wchar_t *enumerator, HWND hwnd, DWORD flags);

static HDEVINFO WINAPI my_SetupDiGetClassDevsA(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags);

static BOOL WINAPI my_SetupDiEnumDeviceInterfaces(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    const GUID *iface_guid,
    DWORD index,
    SP_DEVICE_INTERFACE_DATA *ifd);

static BOOL WINAPI my_SetupDiGetDeviceInterfaceDetailW(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_W *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data);

static BOOL WINAPI my_SetupDiDestroyDeviceInfoList(HDEVINFO dev_info);

static const struct hook_symbol p3io_setupapi_syms[] = {
    {
        .name = "SetupDiGetClassDevsW",
        .patch = my_SetupDiGetClassDevsW,
        .link = (void **) &next_SetupDiGetClassDevsW,
    },
    {
        .name = "SetupDiGetClassDevsA",
        .patch = my_SetupDiGetClassDevsA,
        .link = (void **) &next_SetupDiGetClassDevsA,
    },
    {
#if 0
        .name       = "SetupDiEnumDeviceInfo",
        .patch      = my_SetupDiEnumDeviceInfo,
        .link       = (void **) &next_SetupDiEnumDeviceInfo,
    }, {
#endif
        .name = "SetupDiEnumDeviceInterfaces",
        .patch = my_SetupDiEnumDeviceInterfaces,
        .link = (void **) &next_SetupDiEnumDeviceInterfaces,
    },
    {
        .name = "SetupDiGetDeviceInterfaceDetailW",
        .patch = my_SetupDiGetDeviceInterfaceDetailW,
        .link = (void **) &next_SetupDiGetDeviceInterfaceDetailW,
    },
    {
        .name = "SetupDiDestroyDeviceInfoList",
        .patch = my_SetupDiDestroyDeviceInfoList,
        .link = (void **) &next_SetupDiDestroyDeviceInfoList,
    },
};

/* p3iolib appends \p3io to whatever path is returned by SETUPAPI */

static const wchar_t p3io_path_prefix[] = L"$p3io";
static const wchar_t p3io_path[] = L"$p3io\\p3io";

static HDEVINFO p3io_hdevinfo;

static HDEVINFO WINAPI my_SetupDiGetClassDevsA(
    const GUID *class_guid, const char *enumerator, HWND hwnd, DWORD flags)
{
    HDEVINFO result;

    result = next_SetupDiGetClassDevsA(class_guid, enumerator, hwnd, flags);

    if (result != INVALID_HANDLE_VALUE && IsEqualGUID(class_guid, &p3io_guid)) {
        p3io_hdevinfo = result;
    }

    return result;
}

static HDEVINFO WINAPI my_SetupDiGetClassDevsW(
    const GUID *class_guid, const wchar_t *enumerator, HWND hwnd, DWORD flags)
{
    HDEVINFO result;

    result = next_SetupDiGetClassDevsW(class_guid, enumerator, hwnd, flags);

    if (result != INVALID_HANDLE_VALUE && IsEqualGUID(class_guid, &p3io_guid)) {
        p3io_hdevinfo = result;
    }

    return result;
}

static BOOL WINAPI my_SetupDiEnumDeviceInterfaces(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    const GUID *iface_guid,
    DWORD index,
    SP_DEVICE_INTERFACE_DATA *ifd)
{
    if (dev_info != p3io_hdevinfo) {
        return next_SetupDiEnumDeviceInterfaces(
            dev_info, info_data, iface_guid, index, ifd);
    }

    /* Not implemented */

    log_assert(info_data == NULL);

    /* Class GUID is not the same thing as interface GUID but whatever. p3iolib
       treats them as the same thing. */

    if (index > 0) {
        SetLastError(ERROR_NO_MORE_ITEMS);

        return FALSE;
    }

    if (ifd == NULL || ifd->cbSize != sizeof(*ifd)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);

        return FALSE;
    }

    memcpy(&ifd->InterfaceClassGuid, &p3io_guid, sizeof(GUID));
    ifd->Flags = SPINT_ACTIVE | SPINT_DEFAULT;
    ifd->Reserved = 0;

    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL WINAPI my_SetupDiGetDeviceInterfaceDetailW(
    HDEVINFO dev_info,
    SP_DEVICE_INTERFACE_DATA *ifd,
    SP_DEVICE_INTERFACE_DETAIL_DATA_W *detail,
    DWORD size,
    DWORD *required_size,
    SP_DEVINFO_DATA *info_data)
{
    if (dev_info != p3io_hdevinfo) {
        return next_SetupDiGetDeviceInterfaceDetailW(
            dev_info, ifd, detail, size, required_size, info_data);
    }

    /* Not implemented */

    log_assert(info_data == NULL);

    if (required_size != NULL) {
        *required_size = sizeof(*detail) + sizeof(p3io_path_prefix);
    }

    if (detail != NULL) {
        if (size < sizeof(*detail) + sizeof(p3io_path_prefix)) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);

            return FALSE;
        }

        if (detail->cbSize != sizeof(*detail)) {
            SetLastError(ERROR_INVALID_USER_BUFFER);

            return FALSE;
        }

        memcpy(detail->DevicePath, p3io_path_prefix, sizeof(p3io_path_prefix));
    }

    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL WINAPI my_SetupDiDestroyDeviceInfoList(HDEVINFO dev_info)
{
    if (dev_info == p3io_hdevinfo) {
        p3io_hdevinfo = NULL;
    }

    return next_SetupDiDestroyDeviceInfoList(dev_info);
}

void p3io_setupapi_insert_hooks(HMODULE target)
{
    hook_table_apply(
        target,
        "setupapi.dll",
        p3io_setupapi_syms,
        lengthof(p3io_setupapi_syms));

    log_info("Inserted P3IO setupapi hooks into %p", target);
}

bool p3io_setupapi_match_path(const wchar_t *path)
{
    return wstr_eq(path, p3io_path);
}

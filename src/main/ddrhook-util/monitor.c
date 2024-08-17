// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
// clang-format on

#include <stdbool.h>
#include <wchar.h>

#include "ddrhook-util/monitor.h"

#include "hook/table.h"

#include "iface-core/log.h"

#include "util/defs.h"
#include "util/str.h"

/* Link pointers */

static HDEVINFO(WINAPI *next_SetupDiGetClassDevsW)(
    const GUID *class_guid, const wchar_t *enumerator, HWND hwnd, DWORD flags);

static BOOL(WINAPI *next_SetupDiEnumDeviceInfo)(
    HDEVINFO dev_info, DWORD index, SP_DEVINFO_DATA *info_data);

static BOOL(WINAPI *next_SetupDiGetDeviceRegistryPropertyA)(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    DWORD prop,
    DWORD *reg_type,
    BYTE *bytes,
    DWORD nbytes,
    DWORD *need_nbytes);

static BOOL(WINAPI *next_SetupDiDestroyDeviceInfoList)(HDEVINFO dev_info);

/* API hooks */

static HDEVINFO WINAPI my_SetupDiGetClassDevsW(
    const GUID *class_guid, const wchar_t *enumerator, HWND hwnd, DWORD flags);

static BOOL WINAPI my_SetupDiEnumDeviceInfo(
    HDEVINFO dev_info, DWORD index, SP_DEVINFO_DATA *info_data);

static BOOL WINAPI my_SetupDiGetDeviceRegistryPropertyA(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    DWORD prop,
    DWORD *reg_type,
    BYTE *bytes,
    DWORD nbytes,
    DWORD *need_nbytes);

static BOOL WINAPI my_SetupDiDestroyDeviceInfoList(HDEVINFO dev_info);

static const struct hook_symbol monitor_setupapi_syms[] = {
    {
        .name = "SetupDiGetClassDevsW",
        .patch = my_SetupDiGetClassDevsW,
        .link = (void **) &next_SetupDiGetClassDevsW,
    },
    {
        .name = "SetupDiEnumDeviceInfo",
        .patch = my_SetupDiEnumDeviceInfo,
        .link = (void **) &next_SetupDiEnumDeviceInfo,
    },
    {
        .name = "SetupDiGetDeviceRegistryPropertyA",
        .patch = my_SetupDiGetDeviceRegistryPropertyA,
        .link = (void **) &next_SetupDiGetDeviceRegistryPropertyA,
    },
    {
        .name = "SetupDiDestroyDeviceInfoList",
        .patch = my_SetupDiDestroyDeviceInfoList,
        .link = (void **) &next_SetupDiDestroyDeviceInfoList,
    },
};

extern bool standard_def;
static HDEVINFO monitor_hdevinfo;

static HDEVINFO WINAPI my_SetupDiGetClassDevsW(
    const GUID *class_guid, const wchar_t *enumerator, HWND hwnd, DWORD flags)
{
    HDEVINFO result;

    result = next_SetupDiGetClassDevsW(class_guid, enumerator, hwnd, flags);

    if (result != INVALID_HANDLE_VALUE &&
        IsEqualGUID(class_guid, &monitor_guid)) {
        monitor_hdevinfo = result;
    }

    return result;
}

static BOOL WINAPI my_SetupDiEnumDeviceInfo(
    HDEVINFO dev_info, DWORD index, SP_DEVINFO_DATA *info_data)
{
    if (dev_info != monitor_hdevinfo) {
        return next_SetupDiEnumDeviceInfo(dev_info, index, info_data);
    }

    if (info_data == NULL || info_data->cbSize != sizeof(*info_data)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);

        return FALSE;
    }

    if (index > 0) {
        SetLastError(ERROR_NO_MORE_ITEMS);

        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL WINAPI my_SetupDiGetDeviceRegistryPropertyA(
    HDEVINFO dev_info,
    SP_DEVINFO_DATA *info_data,
    DWORD prop,
    DWORD *reg_type,
    BYTE *bytes,
    DWORD nbytes,
    DWORD *nbytes_out)
{
    const char *txt;
    size_t txt_nbytes;

    if (dev_info != monitor_hdevinfo) {
        return next_SetupDiGetDeviceRegistryPropertyA(
            dev_info, info_data, prop, reg_type, bytes, nbytes, nbytes_out);
    }

    /* The only implemented property */
    log_assert(prop == SPDRP_DEVICEDESC);

    if (standard_def) {
        txt = "Generic Television";
    } else {
        txt = "Generic Monitor";
    }

    txt_nbytes = strlen(txt) + 1;

    if (reg_type != NULL) {
        *reg_type = REG_SZ;
    }

    if (nbytes_out != NULL) {
        *nbytes_out = txt_nbytes;
    }

    if (nbytes < txt_nbytes) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);

        return FALSE;
    }

    if (bytes == NULL) {
        SetLastError(ERROR_INVALID_USER_BUFFER);

        return FALSE;
    }

    str_cpy((char *) bytes, nbytes, txt);
    SetLastError(ERROR_SUCCESS);

    return TRUE;
}

static BOOL WINAPI my_SetupDiDestroyDeviceInfoList(HDEVINFO dev_info)
{
    if (dev_info == monitor_hdevinfo) {
        monitor_hdevinfo = NULL;
    }

    return next_SetupDiDestroyDeviceInfoList(dev_info);
}

void monitor_setupapi_insert_hooks(HMODULE target)
{
    hook_table_apply(
        target,
        "setupapi.dll",
        monitor_setupapi_syms,
        lengthof(monitor_setupapi_syms));

    log_info("Inserted monitor setupapi hooks into %p", target);
}

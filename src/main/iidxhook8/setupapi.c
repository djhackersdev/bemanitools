#define LOG_MODULE "setupapi-hook"

#include <windows.h>
#include <setupapi.h>
#include <initguid.h>

#include "hook/table.h"

#include "iidxhook8/setupapi.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

static BOOL my_SetupDiDestroyDeviceInfoList(
  HDEVINFO DeviceInfoSet
);

static BOOL (*real_SetupDiDestroyDeviceInfoList)(
  HDEVINFO DeviceInfoSet
);

static BOOL my_SetupDiEnumDeviceInfo(
  HDEVINFO         DeviceInfoSet,
  DWORD            MemberIndex,
  PSP_DEVINFO_DATA DeviceInfoData
);

static BOOL (*real_SetupDiEnumDeviceInfo)(
  HDEVINFO         DeviceInfoSet,
  DWORD            MemberIndex,
  PSP_DEVINFO_DATA DeviceInfoData
);

static HKEY my_SetupDiOpenDevRegKey(
  HDEVINFO         DeviceInfoSet,
  PSP_DEVINFO_DATA DeviceInfoData,
  DWORD            Scope,
  DWORD            HwProfile,
  DWORD            KeyType,
  REGSAM           samDesired
);

static HKEY (*real_SetupDiOpenDevRegKey)(
  HDEVINFO         DeviceInfoSet,
  PSP_DEVINFO_DATA DeviceInfoData,
  DWORD            Scope,
  DWORD            HwProfile,
  DWORD            KeyType,
  REGSAM           samDesired
);

static BOOL my_SetupDiGetDeviceRegistryPropertyA(
  HDEVINFO         DeviceInfoSet,
  PSP_DEVINFO_DATA DeviceInfoData,
  DWORD            Property,
  PDWORD           PropertyRegDataType,
  PBYTE            PropertyBuffer,
  DWORD            PropertyBufferSize,
  PDWORD           RequiredSize
);

static BOOL (*real_SetupDiGetDeviceRegistryPropertyA)(
  HDEVINFO         DeviceInfoSet,
  PSP_DEVINFO_DATA DeviceInfoData,
  DWORD            Property,
  PDWORD           PropertyRegDataType,
  PBYTE            PropertyBuffer,
  DWORD            PropertyBufferSize,
  PDWORD           RequiredSize
);

static BOOL my_SetupDiGetDeviceInfoListDetailA(
  HDEVINFO                       DeviceInfoSet,
  PSP_DEVINFO_LIST_DETAIL_DATA_A DeviceInfoSetDetailData
);

static BOOL (*real_SetupDiGetDeviceInfoListDetailA)(
  HDEVINFO                       DeviceInfoSet,
  PSP_DEVINFO_LIST_DETAIL_DATA_A DeviceInfoSetDetailData
);

static HDEVINFO my_SetupDiGetClassDevsA(
  CONST GUID *ClassGuid,
  PCSTR     Enumerator,
  HWND       hwndParent,
  DWORD      Flags
);

static HDEVINFO(*real_SetupDiGetClassDevsA)(
  CONST GUID *ClassGuid,
  PCSTR     Enumerator,
  HWND       hwndParent,
  DWORD      Flags
);

static const struct hook_symbol iidxhook5_setupapi_syms[] = {
    {
        .name   = "SetupDiDestroyDeviceInfoList",
        .patch  = my_SetupDiDestroyDeviceInfoList,
        .link   = (void **) &real_SetupDiDestroyDeviceInfoList
    },
    {
        .name   = "SetupDiEnumDeviceInfo",
        .patch  = my_SetupDiEnumDeviceInfo,
        .link   = (void **) &real_SetupDiEnumDeviceInfo
    },
    {
        .name   = "SetupDiOpenDevRegKey",
        .patch  = my_SetupDiOpenDevRegKey,
        .link   = (void **) &real_SetupDiOpenDevRegKey
    },
    {
        .name   = "SetupDiGetDeviceRegistryPropertyA",
        .patch  = my_SetupDiGetDeviceRegistryPropertyA,
        .link   = (void **) &real_SetupDiGetDeviceRegistryPropertyA
    },
    {
        .name   = "SetupDiGetDeviceInfoListDetailA",
        .patch  = my_SetupDiGetDeviceInfoListDetailA,
        .link   = (void **) &real_SetupDiGetDeviceInfoListDetailA
    },
    {
        .name   = "SetupDiGetClassDevsA",
        .patch  = my_SetupDiGetClassDevsA,
        .link   = (void **) &real_SetupDiGetClassDevsA
    },
};

static LSTATUS my_RegQueryValueExA(
  HKEY                              hKey,
  LPCSTR                            lpValueName,
  LPDWORD                           lpReserved,
  LPDWORD                           lpType,
  LPBYTE                            lpData,
  LPDWORD                           lpcbData
);
static LSTATUS (*real_RegQueryValueExA)(
  HKEY                              hKey,
  LPCSTR                            lpValueName,
  LPDWORD                           lpReserved,
  LPDWORD                           lpType,
  LPBYTE                            lpData,
  LPDWORD                           lpcbData
);

static const struct hook_symbol iidxhook5_Advapi32_syms[] = {
    {
        .name   = "RegQueryValueExA",
        .patch  = my_RegQueryValueExA,
        .link   = (void **) &real_RegQueryValueExA
    },
};

#define CUSTOM_DEVICE_HANDLE (void*)0x12341230
#define CUSTOM_REGISTRY_HANDLE (void*)0x4242ccc0

static BOOL my_SetupDiDestroyDeviceInfoList(
  HDEVINFO DeviceInfoSet
){
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE){
        log_info("Inside: %s", __FUNCTION__);
        return true;
    }
    return real_SetupDiDestroyDeviceInfoList(DeviceInfoSet);
}

static BOOL my_SetupDiEnumDeviceInfo(
  HDEVINFO         DeviceInfoSet,
  DWORD            MemberIndex,
  PSP_DEVINFO_DATA DeviceInfoData
){
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE){
        log_info("Inside: %s", __FUNCTION__);
        return true;
    }
    return real_SetupDiEnumDeviceInfo(DeviceInfoSet, MemberIndex, DeviceInfoData);
}

static HKEY my_SetupDiOpenDevRegKey(
  HDEVINFO         DeviceInfoSet,
  PSP_DEVINFO_DATA DeviceInfoData,
  DWORD            Scope,
  DWORD            HwProfile,
  DWORD            KeyType,
  REGSAM           samDesired
){
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE){
        log_info("Inside: %s", __FUNCTION__);
        return CUSTOM_REGISTRY_HANDLE;
    }
    return real_SetupDiOpenDevRegKey(DeviceInfoSet, DeviceInfoData, Scope, HwProfile, KeyType, samDesired);
}

static BOOL my_SetupDiGetDeviceRegistryPropertyA(
  HDEVINFO         DeviceInfoSet,
  PSP_DEVINFO_DATA DeviceInfoData,
  DWORD            Property,
  PDWORD           PropertyRegDataType,
  PBYTE            PropertyBuffer,
  DWORD            PropertyBufferSize,
  PDWORD           RequiredSize
){
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE){
        log_info("Inside: %s", __FUNCTION__);
        log_info("%s: Found CUSTOM HANDLE", __FUNCTION__);
        if (PropertyBuffer && (PropertyBufferSize >= 12)){
            log_info("%s: Copying property name (%ld)", __FUNCTION__, PropertyBufferSize);
            strncpy((char*)PropertyBuffer, "BIO2(VIDEO)", PropertyBufferSize);
            log_info("%s: Done copying property name", __FUNCTION__);
            // Sleep(500);
        } else {
            log_info("%s: Returning size", __FUNCTION__);
            *RequiredSize = 12;
        }
        log_info("%s: STUB RETURN", __FUNCTION__);
        return true;
    }
    return real_SetupDiGetDeviceRegistryPropertyA(DeviceInfoSet, DeviceInfoData, Property, PropertyRegDataType, PropertyBuffer, PropertyBufferSize,  RequiredSize);
}

static BOOL my_SetupDiGetDeviceInfoListDetailA(
  HDEVINFO                       DeviceInfoSet,
  PSP_DEVINFO_LIST_DETAIL_DATA_A DeviceInfoSetDetailData
){
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE){
        log_info("Inside: %s", __FUNCTION__);
        return true;
    }
    return real_SetupDiGetDeviceInfoListDetailA(DeviceInfoSet, DeviceInfoSetDetailData);
}

DEFINE_GUID(GUID_COM_BUS_ENUMERATOR,
  0x4D36E978, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);

static HDEVINFO my_SetupDiGetClassDevsA(
  CONST GUID *ClassGuid,
  PCSTR     Enumerator,
  HWND       hwndParent,
  DWORD      Flags
){
    if (ClassGuid) {
        if (IsEqualGUID(ClassGuid, &GUID_COM_BUS_ENUMERATOR)){
            log_info("Inside: %s", __FUNCTION__);
            return CUSTOM_DEVICE_HANDLE;
        }
    }
    return real_SetupDiGetClassDevsA(ClassGuid, Enumerator, hwndParent, Flags);
}

static LSTATUS my_RegQueryValueExA(
  HKEY                              hKey,
  LPCSTR                            lpValueName,
  LPDWORD                           lpReserved,
  LPDWORD                           lpType,
  LPBYTE                            lpData,
  LPDWORD                           lpcbData
){
    if (hKey == CUSTOM_REGISTRY_HANDLE){
        if (strcmp(lpValueName, "PortName") == 0) {
            log_info("Inside: %s", __FUNCTION__);
            if (lpData){
                strncpy((char*)lpData, "COM4", *lpcbData);
                return ERROR_SUCCESS;
            }
            return ERROR_MORE_DATA;
        }
    }
    return real_RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

void setupapi_hook_init(void)
{
    hook_table_apply(
            NULL,
            "setupapi.dll",
            iidxhook5_setupapi_syms,
            lengthof(iidxhook5_setupapi_syms));
    hook_table_apply(
            NULL,
            "Advapi32.dll",
            iidxhook5_Advapi32_syms,
            lengthof(iidxhook5_Advapi32_syms));

    log_info("Inserted setupapi hooks");
}

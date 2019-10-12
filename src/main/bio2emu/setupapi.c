#define LOG_MODULE "setupapi-hook"

#include <windows.h>
#include <setupapi.h>
#include <initguid.h>

#include "hook/table.h"

#include "bio2emu/setupapi.h"
#include "bio2emu/emu.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

#define MAX_INSTANCES_HOOKED 8

static struct array* bio2_assigned_ports = NULL;

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

static const struct hook_symbol bio2emu_setupapi_syms[] = {
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

static const struct hook_symbol bio2emu_Advapi32_syms[] = {
    {
        .name   = "RegQueryValueExA",
        .patch  = my_RegQueryValueExA,
        .link   = (void **) &real_RegQueryValueExA
    },
};

static void* CUSTOM_DEVICE_HANDLE;
static struct HKEY__ CUSTOM_REGISTRY_HANDLE[MAX_INSTANCES_HOOKED];

static BOOL check_if_match(HKEY ptr, HKEY base) {
    return (ptr >= &base[0]) && (ptr <= &base[MAX_INSTANCES_HOOKED - 1]);
}

static size_t get_match_index(HKEY ptr, HKEY base) {
    for (size_t i = 0; i < MAX_INSTANCES_HOOKED; ++i) {
        if (ptr == &base[i]) {
            return i;
        }
    }
    return -1;
}

static BOOL my_SetupDiDestroyDeviceInfoList(
    HDEVINFO DeviceInfoSet
){
    if (DeviceInfoSet == &CUSTOM_DEVICE_HANDLE){
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
    if (DeviceInfoSet == &CUSTOM_DEVICE_HANDLE){
        log_info("%s: Loaded idx %ld", __FUNCTION__, MemberIndex);
        if (MemberIndex < MAX_INSTANCES_HOOKED) {
            DeviceInfoData->DevInst = MemberIndex;
            return true;
        }
        return false;
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
    if (DeviceInfoSet == &CUSTOM_DEVICE_HANDLE){
        if (DeviceInfoData->DevInst < MAX_INSTANCES_HOOKED){
            log_info("%s: matched instance", __FUNCTION__);
            return &CUSTOM_REGISTRY_HANDLE[DeviceInfoData->DevInst];
        }
    }
    return real_SetupDiOpenDevRegKey(DeviceInfoSet, DeviceInfoData, Scope, HwProfile, KeyType, samDesired);
}

static const char DEVICE_PROPERTY_VALUE[] = "BIO2(VIDEO)(";
static const size_t DEVICE_PROPERTY_LENGTH = 12;

static BOOL my_SetupDiGetDeviceRegistryPropertyA(
    HDEVINFO         DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    DWORD            Property,
    PDWORD           PropertyRegDataType,
    PBYTE            PropertyBuffer,
    DWORD            PropertyBufferSize,
    PDWORD           RequiredSize
){
    if (DeviceInfoSet == &CUSTOM_DEVICE_HANDLE){
        if (DeviceInfoData->DevInst< MAX_INSTANCES_HOOKED){
            struct bio2emu_port* selected_port = *array_item(struct bio2emu_port*, bio2_assigned_ports, DeviceInfoData->DevInst);
            size_t portname_len = strlen(selected_port->port);
            size_t required_size = (DEVICE_PROPERTY_LENGTH + portname_len + 1 + 1); // + 1 for ')', + 1 for NULL

            if (PropertyBuffer && (PropertyBufferSize >= required_size)){
                char* PropBuffStr = (char*)PropertyBuffer;

                strcpy(PropBuffStr, DEVICE_PROPERTY_VALUE);
                strcpy(PropBuffStr + DEVICE_PROPERTY_LENGTH, selected_port->port);
                strcpy(PropBuffStr + DEVICE_PROPERTY_LENGTH + portname_len, ")");

                log_info("%s: Done copying property name [%s]", __FUNCTION__, PropBuffStr);
            } else {
                log_info("%s: Returning size", __FUNCTION__);
                *RequiredSize = required_size;
            }
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
    if (DeviceInfoSet == &CUSTOM_DEVICE_HANDLE){
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
            return &CUSTOM_DEVICE_HANDLE;
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
    if (check_if_match(hKey, CUSTOM_REGISTRY_HANDLE)){
        if (strcmp(lpValueName, "PortName") == 0) {
            if (lpData){
                size_t portidx = get_match_index(hKey, CUSTOM_REGISTRY_HANDLE);
                struct bio2emu_port* selected_port = *array_item(struct bio2emu_port*, bio2_assigned_ports, portidx);
                strncpy((char*)lpData, selected_port->port, *lpcbData);

                log_info("%s: Queried %s", __FUNCTION__, selected_port->port);
                return ERROR_SUCCESS;
            }
            return ERROR_MORE_DATA;
        }
    }
    return real_RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

void bio2emu_setupapi_hook_init(struct array* bio2_ports)
{
    bio2_assigned_ports = bio2_ports;

    hook_table_apply(
            NULL,
            "setupapi.dll",
            bio2emu_setupapi_syms,
            lengthof(bio2emu_setupapi_syms));

    hook_table_apply(
            NULL,
            "Advapi32.dll",
            bio2emu_Advapi32_syms,
            lengthof(bio2emu_Advapi32_syms));

    log_info("Inserted setupapi hooks");
}

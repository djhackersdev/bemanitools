#define LOG_MODULE "cam-hook"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <initguid.h>
// clang-format on

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>

#include <cfgmgr32.h>
#include <setupapi.h>

#include <stdio.h>

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "camhook/cam.h"
#include "camhook/cam-detect.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"

static struct CameraData camData[CAMHOOK_CONFIG_CAM_MAX];
int camAddresses[CAMHOOK_CONFIG_CAM_MAX] = {
    1,
    7,
};
static size_t num_addressed_cams = 0;
static size_t num_located_cams = 0;

static enum camhook_version camhook_version = CAMHOOK_OLD;

static CONFIGRET my_CM_Locate_DevNodeA(
    PDEVINST pdnDevInst, DEVINSTID_A pDeviceID, ULONG ulFlags);

static CONFIGRET
my_CM_Get_Parent(PDEVINST pdnDevInst, DEVINST dnDevInst, ULONG ulFlags);

static CONFIGRET my_CM_Get_Device_IDA(
    DEVINST dnDevInst, PSTR Buffer, ULONG BufferLen, ULONG ulFlags);

static CONFIGRET (*real_CM_Locate_DevNodeA)(
    PDEVINST pdnDevInst, DEVINSTID_A pDeviceID, ULONG ulFlags);

static CONFIGRET (*real_CM_Get_Parent)(
    PDEVINST pdnDevInst, DEVINST dnDevInst, ULONG ulFlags);

static CONFIGRET (*real_CM_Get_Device_IDA)(
    DEVINST dnDevInst, PSTR Buffer, ULONG BufferLen, ULONG ulFlags);

static HRESULT my_MFEnumDeviceSources(
    IMFAttributes *pAttributes,
    IMFActivate ***pppSourceActivate,
    UINT32 *pcSourceActivate);

static HRESULT (*real_MFEnumDeviceSources)(
    IMFAttributes *pAttributes,
    IMFActivate ***pppSourceActivate,
    UINT32 *pcSourceActivate);

static BOOL my_SetupDiDestroyDeviceInfoList(HDEVINFO DeviceInfoSet);

static BOOL (*real_SetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);

static BOOL my_SetupDiEnumDeviceInfo(
    HDEVINFO DeviceInfoSet, DWORD MemberIndex, PSP_DEVINFO_DATA DeviceInfoData);

static BOOL (*real_SetupDiEnumDeviceInfo)(
    HDEVINFO DeviceInfoSet, DWORD MemberIndex, PSP_DEVINFO_DATA DeviceInfoData);

static BOOL my_SetupDiGetDeviceRegistryPropertyA(
    HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    DWORD Property,
    PDWORD PropertyRegDataType,
    PBYTE PropertyBuffer,
    DWORD PropertyBufferSize,
    PDWORD RequiredSize);

static BOOL (*real_SetupDiGetDeviceRegistryPropertyA)(
    HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    DWORD Property,
    PDWORD PropertyRegDataType,
    PBYTE PropertyBuffer,
    DWORD PropertyBufferSize,
    PDWORD RequiredSize);

static HDEVINFO my_SetupDiGetClassDevsA(
    CONST GUID *ClassGuid, PCSTR Enumerator, HWND hwndParent, DWORD Flags);

static HDEVINFO (*real_SetupDiGetClassDevsA)(
    CONST GUID *ClassGuid, PCSTR Enumerator, HWND hwndParent, DWORD Flags);

static const struct hook_symbol camhook_cfgmgr32_syms[] = {
    {.name = "CM_Locate_DevNodeA",
     .patch = my_CM_Locate_DevNodeA,
     .link = (void **) &real_CM_Locate_DevNodeA},
    {.name = "CM_Get_Parent",
     .patch = my_CM_Get_Parent,
     .link = (void **) &real_CM_Get_Parent},
    {.name = "CM_Get_Device_IDA",
     .patch = my_CM_Get_Device_IDA,
     .link = (void **) &real_CM_Get_Device_IDA},
    {.name = "SetupDiDestroyDeviceInfoList",
     .patch = my_SetupDiDestroyDeviceInfoList,
     .link = (void **) &real_SetupDiDestroyDeviceInfoList},
    {.name = "SetupDiEnumDeviceInfo",
     .patch = my_SetupDiEnumDeviceInfo,
     .link = (void **) &real_SetupDiEnumDeviceInfo},
    {.name = "SetupDiGetDeviceRegistryPropertyA",
     .patch = my_SetupDiGetDeviceRegistryPropertyA,
     .link = (void **) &real_SetupDiGetDeviceRegistryPropertyA},
    {.name = "SetupDiGetClassDevsA",
     .patch = my_SetupDiGetClassDevsA,
     .link = (void **) &real_SetupDiGetClassDevsA},
};

static const struct hook_symbol camhook_mf_syms[] = {
    {.name = "MFEnumDeviceSources",
     .patch = my_MFEnumDeviceSources,
     .link = (void **) &real_MFEnumDeviceSources},
};

DEVINST camhook_custom_nodes[CAMHOOK_CONFIG_CAM_MAX] = {
    0x04040004,
    0x04040008,
};
DEVINST camhook_custom_parent_nodes[CAMHOOK_CONFIG_CAM_MAX] = {
    0x04040014,
    0x04040018,
};
const char *camhook_custom_parent_device_id[CAMHOOK_CONFIG_CAM_MAX] = {
    "USB\\VEN_1022&DEV_7908",
    "USB\\VEN_1022&DEV_7914",
};

static CONFIGRET
my_CM_Locate_DevNodeA(PDEVINST pdnDevInst, DEVINSTID_A pDeviceID, ULONG ulFlags)
{
    log_info("Inside: %s", __FUNCTION__);

    char builtString[CAMERA_DATA_STRING_SIZE] = {0};
    if (pdnDevInst) {
        log_info("seeking: %s", pDeviceID);
        for (size_t i = 0; i < CAMHOOK_CONFIG_CAM_MAX; ++i) {
            if (camData[i].setup) {
                snprintf(
                    builtString,
                    CAMERA_DATA_STRING_SIZE,
                    "USB\\VID_288C&PID_0002&MI_00\\%s",
                    camData[i].extra_upper);
                log_info("built: %s", builtString);
                if (strcmp(pDeviceID, builtString) == 0) {
                    if (!camData[i].fake_located) {
                        camData[i].fake_located_node = num_located_cams;
                        camData[i].fake_located = true;
                        ++num_located_cams;
                    }
                    log_info(
                        "Injecting custom device %d to node %x",
                        (int) i,
                        (int) camData[i].fake_located_node);
                    *pdnDevInst =
                        camhook_custom_nodes[camData[i].fake_located_node];
                    return CR_SUCCESS;
                }
            }
        }
    }
    return real_CM_Locate_DevNodeA(pdnDevInst, pDeviceID, ulFlags);
}

static CONFIGRET
my_CM_Get_Parent(PDEVINST pdnDevInst, DEVINST dnDevInst, ULONG ulFlags)
{
    log_info("Inside: %s", __FUNCTION__);

    if (pdnDevInst) {
        for (size_t i = 0; i < CAMHOOK_CONFIG_CAM_MAX; ++i) {
            if (dnDevInst == camhook_custom_nodes[i]) {
                log_info("Injecting custom parent %d", (int) i);
                *pdnDevInst = camhook_custom_parent_nodes[i];
                return CR_SUCCESS;
            }
        }
    }

    return real_CM_Get_Parent(pdnDevInst, dnDevInst, ulFlags);
}

static CONFIGRET my_CM_Get_Device_IDA(
    DEVINST dnDevInst, PSTR Buffer, ULONG BufferLen, ULONG ulFlags)
{
    log_info("Inside: %s", __FUNCTION__);
    if (Buffer) {
        for (size_t i = 0; i < CAMHOOK_CONFIG_CAM_MAX; ++i) {
            if (dnDevInst == camhook_custom_parent_nodes[i]) {
                log_info("Injecting custom parent %d ID", (int) i);
                strncpy(Buffer, camhook_custom_parent_device_id[i], BufferLen);
                Buffer[BufferLen - 1] = '\0';
                log_info("%s", Buffer);
                return CR_SUCCESS;
            }
        }
    }
    return real_CM_Get_Device_IDA(dnDevInst, Buffer, BufferLen, ulFlags);
}

static HRESULT(STDCALL *real_GetAllocatedString)(
    IMFActivate *self, REFGUID guidKey, LPWSTR *ppwszValue, UINT32 *pcchLength);

HRESULT my_GetAllocatedString(
    IMFActivate *self, REFGUID guidKey, LPWSTR *ppwszValue, UINT32 *pcchLength)
{
    HRESULT ret;
    log_info("Inside: %s", __FUNCTION__);

    // should probably check GUID == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, oh well
    ret = real_GetAllocatedString(self, guidKey, ppwszValue, pcchLength);
    char *pMBBuffer = (char *) malloc(0x100);
    wcstombs(pMBBuffer, *ppwszValue, 0x100);
    log_info("Obtained: %s", pMBBuffer);

    wchar_t *pwc = NULL;

    // look for a matching deviceSymbolicLink
    for (size_t i = 0; i < CAMHOOK_CONFIG_CAM_MAX; ++i) {
        if (!pwc) {
            if (camData[i].setup) {
                pwc = wcsstr(*ppwszValue, camData[i].deviceSymbolicLink);
            }
        }
    }

    // if matches, replace with target device ID
    if (pwc) {
        // \\?\usb#vid_288c&pid_0002&mi_00
        pwc[12] = L'2';
        pwc[13] = L'8';
        pwc[14] = L'8';
        pwc[15] = L'c';

        pwc[21] = L'0';
        pwc[22] = L'0';
        pwc[23] = L'0';
        pwc[24] = L'2';

        pwc[29] = L'0';
        pwc[30] = L'0';

        wcstombs(pMBBuffer, *ppwszValue, 0x100);
        log_info("Replaced: %s", pMBBuffer);
    }
    free(pMBBuffer);

    return ret;
}

static HRESULT my_MFEnumDeviceSources(
    IMFAttributes *pAttributes,
    IMFActivate ***pppSourceActivate,
    UINT32 *pcSourceActivate)
{
    IMFActivate *api;
    IMFActivateVtbl *api_vtbl;
    struct com_proxy *api_proxy;

    UINT32 nsrcs;

    HRESULT ret;

    log_info("Inside: %s", __FUNCTION__);
    ret = real_MFEnumDeviceSources(
        pAttributes, pppSourceActivate, pcSourceActivate);
    nsrcs = *pcSourceActivate;

    for (UINT32 i = 0; i < nsrcs; ++i) {
        api = (*pppSourceActivate)[i];

        ret = com_proxy_wrap(&api_proxy, api, sizeof(*api->lpVtbl));

        if (ret != S_OK) {
            log_warning("Wrapping com proxy failed: %08lx", ret);
            return ret;
        }

        api_vtbl = api_proxy->vptr;

        real_GetAllocatedString = api_vtbl->GetAllocatedString;
        api_vtbl->GetAllocatedString = my_GetAllocatedString;

        (*pppSourceActivate)[i] = (IMFActivate *) api_proxy;
    }

    return ret;
}

#define CUSTOM_DEVICE_HANDLE (void *) 0x04041110
static HDEVINFO RealReplacedHandle;

static BOOL my_SetupDiDestroyDeviceInfoList(HDEVINFO DeviceInfoSet)
{
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE) {
        log_info("Inside: %s", __FUNCTION__);
        DeviceInfoSet = RealReplacedHandle;
        RealReplacedHandle = NULL;

        return real_SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

    return real_SetupDiDestroyDeviceInfoList(DeviceInfoSet);
}

static BOOL my_SetupDiEnumDeviceInfo(
    HDEVINFO DeviceInfoSet, DWORD MemberIndex, PSP_DEVINFO_DATA DeviceInfoData)
{
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE) {
        // log_info("Inside: %s", __FUNCTION__);
        return real_SetupDiEnumDeviceInfo(
            RealReplacedHandle, MemberIndex, DeviceInfoData);
    }

    return real_SetupDiEnumDeviceInfo(
        DeviceInfoSet, MemberIndex, DeviceInfoData);
}

static BOOL my_SetupDiGetDeviceRegistryPropertyA(
    HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    DWORD Property,
    PDWORD PropertyRegDataType,
    PBYTE PropertyBuffer,
    DWORD PropertyBufferSize,
    PDWORD RequiredSize)
{
    if (DeviceInfoSet == CUSTOM_DEVICE_HANDLE) {
        BOOL ret = real_SetupDiGetDeviceRegistryPropertyA(
            RealReplacedHandle,
            DeviceInfoData,
            Property,
            PropertyRegDataType,
            PropertyBuffer,
            PropertyBufferSize,
            RequiredSize);

        if (Property == SPDRP_DEVICEDESC) {
            if (PropertyBuffer) {
                char *PropertyBufferChar = (char *) PropertyBuffer;
                for (size_t i = 0; i < CAMHOOK_CONFIG_CAM_MAX; ++i) {
                    if (camData[i].setup) {
                        if (strcmp(
                                PropertyBufferChar, camData[i].parent_name) ==
                            0) {
                            log_info(
                                "%s: replacing %s",
                                __FUNCTION__,
                                camData[i].parent_name);
                            strncpy(
                                PropertyBufferChar,
                                "USB Composite Device",
                                PropertyBufferSize);
                        }
                    }
                }
            }

            return ret;
        } else if (Property == SPDRP_ADDRESS) {
            if (PropertyBuffer) {
                int addr = *(int *) PropertyBuffer;
                for (size_t i = 0; i < CAMHOOK_CONFIG_CAM_MAX; ++i) {
                    if (camData[i].setup) {
                        if (addr == camData[i].parent_address) {
                            if (!camData[i].fake_addressed) {
                                camData[i].fake_address =
                                    camAddresses[num_addressed_cams];
                                camData[i].fake_addressed = true;
                                ++num_addressed_cams;
                            }
                            log_info(
                                "%s: assigning cam %d to addr %d",
                                __FUNCTION__,
                                (int) i,
                                camData[i].fake_address);
                            *(int *) PropertyBuffer = camData[i].fake_address;
                        }
                    }
                }
            }

            return ret;
        }
    }

    return real_SetupDiGetDeviceRegistryPropertyA(
        DeviceInfoSet,
        DeviceInfoData,
        Property,
        PropertyRegDataType,
        PropertyBuffer,
        PropertyBufferSize,
        RequiredSize);
}

static HDEVINFO my_SetupDiGetClassDevsA(
    CONST GUID *ClassGuid, PCSTR Enumerator, HWND hwndParent, DWORD Flags)
{
    if (ClassGuid == NULL && Enumerator != NULL && hwndParent == NULL &&
        Flags == (DIGCF_PRESENT | DIGCF_ALLCLASSES)) {
        if (RealReplacedHandle) {
            log_info("Replacement handle is already set?");
        }

        if (strcmp(Enumerator, "USB") == 0) {
            log_info("Inside: %s", __FUNCTION__);
            RealReplacedHandle = real_SetupDiGetClassDevsA(
                ClassGuid, Enumerator, hwndParent, Flags);

            if (RealReplacedHandle == INVALID_HANDLE_VALUE) {
                return INVALID_HANDLE_VALUE;
            }

            return CUSTOM_DEVICE_HANDLE;
        }
    }
    return real_SetupDiGetClassDevsA(ClassGuid, Enumerator, hwndParent, Flags);
}

void camhook_set_version(enum camhook_version version) {
    camhook_version = version;
}

void camhook_init(struct camhook_config_cam *config_cam)
{
    MFStartup(0x20070u, 0);

    // fill before applying hooks
    for (size_t i = 0; i < config_cam->num_devices; ++i) {
        // Check if this camera is disabled first
        if (config_cam->disable_camera[i]) {
            // If so, pretend this camera is already assigned and move on
            num_addressed_cams++;
            num_located_cams++;
            continue;
        }
        // If not, try to hook the camera
        fill_cam_struct(&camData[i], config_cam->device_id[i]);
    }

    size_t num_setup = 0;

    for (size_t i = 0; i < config_cam->num_devices; ++i) {
        if (camData[i].setup) {
            num_setup++;
        }
    }

    if (num_setup > 0) {
        hook_table_apply(
            NULL,
            "setupapi.dll",
            camhook_cfgmgr32_syms,
            lengthof(camhook_cfgmgr32_syms));
        hook_table_apply(
            NULL, "Mf.dll", camhook_mf_syms, lengthof(camhook_mf_syms));

        log_info("Inserted cam hooks for %d cams", (int) num_setup);
        // If the user has manually disabled all cams, don't print this in the
        // log
    } else if (num_addressed_cams != config_cam->num_devices) {
        log_info("No cams detected, not hooking");
    }
}

void camhook_fini(void)
{
    MFShutdown();
}

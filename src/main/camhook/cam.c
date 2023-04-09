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

#include "util/defs.h"
#include "util/log.h"
#include "util/str.h"
#include "util/time.h"

#define CAMERA_DATA_STRING_SIZE 0x100

EXTERN_GUID(
    MY_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
    0xc60ac5fe,
    0x252a,
    0x478f,
    0xa0,
    0xef,
    0xbc,
    0x8f,
    0xa5,
    0xf7,
    0xca,
    0xd3);
EXTERN_GUID(
    MY_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
    0x8ac3587a,
    0x4ae7,
    0x42d8,
    0x99,
    0xe0,
    0x0a,
    0x60,
    0x13,
    0xee,
    0xf9,
    0x0f);
EXTERN_GUID(
    MY_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
    0x58f0aad8,
    0x22bf,
    0x4f8a,
    0xbb,
    0x3d,
    0xd2,
    0xc4,
    0x97,
    0x8c,
    0x6e,
    0x2f);
// define ourselves cause mingw has these wrong

struct CameraData {
    bool setup;
    char name[CAMERA_DATA_STRING_SIZE];
    char deviceInstancePath[CAMERA_DATA_STRING_SIZE];
    wchar_t deviceSymbolicLink[CAMERA_DATA_STRING_SIZE];
    char extra_upper[CAMERA_DATA_STRING_SIZE];
    int address;
    char parent_name[CAMERA_DATA_STRING_SIZE];
    char parent_deviceInstancePath[CAMERA_DATA_STRING_SIZE];
    int parent_address;

    bool fake_addressed;
    int fake_address;

    bool fake_located;
    size_t fake_located_node;
};

static struct CameraData camData[CAMHOOK_CONFIG_CAM_MAX];
int camAddresses[CAMHOOK_CONFIG_CAM_MAX] = {
    1,
    7,
};
static size_t num_addressed_cams = 0;
static size_t num_located_cams = 0;

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

    // should probably check GUID, oh well
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

bool check_four(const char inA[4], const char inB[4])
{
    return (*(uint32_t *) inA == *(uint32_t *) inB);
}

char *grab_next_camera_id(char *buffer, size_t bsz)
{
    static size_t gotten = 0;

    IMFAttributes *pAttributes = NULL;
    IMFActivate **ppDevices = NULL;

    buffer[0] = '\0';

    HRESULT hr = MFCreateAttributes(&pAttributes, 1);

    if (FAILED(hr)) {
        log_info("MFCreateAttributes failed: %ld", hr);
        goto done;
    }

    hr = pAttributes->lpVtbl->SetGUID(
        pAttributes,
        &MY_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        &MY_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) {
        log_info("SetGUID failed: %ld", hr);
        goto done;
    }

    UINT32 count;
    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);

    if (FAILED(hr)) {
        log_info("MFEnumDeviceSources failed: %ld", hr);
        goto done;
    }

    if (count <= gotten) {
        log_info("gotten failed: %d < %d", count, (int) gotten);
        // not enough remaining
        goto done;
    }

    wchar_t wSymLink[CAMERA_DATA_STRING_SIZE];
    UINT32 sz;

    hr = ppDevices[gotten]->lpVtbl->GetString(
        ppDevices[gotten],
        &MY_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
        wSymLink,
        CAMERA_DATA_STRING_SIZE,
        &sz);

    if (FAILED(hr)) {
        log_info("GetString failed: %ld", hr);
        goto done;
    }

    wcstombs(buffer, wSymLink, bsz);
    log_info("Detected webcam: %s\n", buffer);
    ++gotten;

done:
    if (pAttributes) {
        pAttributes->lpVtbl->Release(pAttributes);
    }

    for (DWORD i = 0; i < count; i++) {
        if (ppDevices != NULL && ppDevices[i]) {
            ppDevices[i]->lpVtbl->Release(ppDevices[i]);
        }
    }

    CoTaskMemFree(ppDevices);

    return buffer;
}

bool convert_sym_to_path(const char *sym, char *path)
{
    HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);

    if (DeviceInfoSet == INVALID_HANDLE_VALUE) {
        log_info("Could not open SetupDiCreateDeviceInfoList\n");
        return 0;
    }

    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData = {0};
    DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if (!SetupDiOpenDeviceInterfaceA(
            DeviceInfoSet, sym, 0, &DeviceInterfaceData)) {
        log_info("Could not SetupDiOpenDeviceInterfaceA\n");
        return 0;
    }

    SP_DEVINFO_DATA DeviceInfoData = {0};
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if (!SetupDiGetDeviceInterfaceDetailA(
            DeviceInfoSet,
            &DeviceInterfaceData,
            NULL,
            0,
            NULL,
            &DeviceInfoData)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            log_info("Could not SetupDiGetDeviceInterfaceDetailA\n");
            return 0;
        }
    }

    DWORD sz;

    if (!SetupDiGetDeviceInstanceIdA(
            DeviceInfoSet,
            &DeviceInfoData,
            path,
            CAMERA_DATA_STRING_SIZE,
            &sz)) {
        log_info("Could not SetupDiGetDeviceInstanceIdA\n");
        return 0;
    }

    if (DeviceInfoSet != INVALID_HANDLE_VALUE) {
        if (!SetupDiDeleteDeviceInterfaceData(
                DeviceInfoSet, &DeviceInterfaceData)) {
            log_info("Could not SetupDiDeleteDeviceInterfaceData\n");
            return 0;
        }

        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

    return 1;
}

void strtolower(char *str)
{
    for (size_t i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

bool convert_path_to_fakesym(const char *path, wchar_t *sym, char *extra_o)
{
    char root[16] = {0};
    char vidstr[16] = {0};
    char pidstr[16] = {0};
    char mistr[16] = {0};
    char extra[64] = {0};
    sscanf(
        path,
        "%[^\\]\\%[^&]&%[^&]&%[^\\]\\%s",
        root,
        vidstr,
        pidstr,
        mistr,
        extra);
    strcpy(extra_o, extra);

    strtolower(root);
    strtolower(vidstr);
    strtolower(pidstr);
    strtolower(mistr);
    strtolower(extra);

    char buffer[CAMERA_DATA_STRING_SIZE];
    snprintf(
        buffer,
        CAMERA_DATA_STRING_SIZE,
        "\\\\?\\%s#%s&%s&%s#%s",
        root,
        vidstr,
        pidstr,
        mistr,
        extra);
    
    mbstowcs(sym, buffer, CAMERA_DATA_STRING_SIZE);

    return true;
}

void fill_cam_struct(struct CameraData *data, const char *devid)
{
    char buffer[CAMERA_DATA_STRING_SIZE];

    data->setup = false;

    if (!devid || strlen(devid) == 0) {
        devid = grab_next_camera_id(buffer, CAMERA_DATA_STRING_SIZE);
    }

    if (!devid || strlen(devid) == 0) {
        // no more cameras remain?
        return;
    }

    if (strlen(devid) >= CAMERA_DATA_STRING_SIZE) {
        // error probably log something?
        return;
    }

    // detect input type
    if (check_four(devid, "\\\\?\\")) {
        // SYMBOLIC_LINK
        if (!convert_sym_to_path(devid, data->deviceInstancePath)) {
            log_info("Could not convert %s to path", devid);
            return;
        }
    } else if (check_four(devid, "USB\\")) {
        // Device instance path
        strcpy(data->deviceInstancePath, devid);
        // continue
    } else if (check_four(devid, "SKIP")) {
        // User wants to leave this camera unassigned
        num_addressed_cams++;
        num_located_cams++;
        return;
    } else {
        // UNKNOWN ENTRY
        log_info("UNK: %s", devid);
        log_info("Please enter the device instance path");
        return;
    }

    if (!convert_path_to_fakesym(
            data->deviceInstancePath,
            data->deviceSymbolicLink,
            data->extra_upper)) {
        log_info("Could not convert %s to sym", data->deviceInstancePath);
        return;
    }

    log_info("dev path: %s", data->deviceInstancePath);

    // locate device nodes
    DEVINST dnDevInst;
    DEVINST parentDev;
    CONFIGRET cmret;

    cmret = CM_Locate_DevNodeA(
        &dnDevInst, data->deviceInstancePath, CM_LOCATE_DEVNODE_NORMAL);

    if (cmret != CR_SUCCESS) {
        log_info("CM_Locate_DevNodeA fail: %s", data->deviceInstancePath);
        return;
    }

    cmret = CM_Get_Parent(&parentDev, dnDevInst, 0);

    if (cmret != CR_SUCCESS) {
        log_info("CM_Get_Parent fail: %s", data->deviceInstancePath);
        return;
    }
    cmret = CM_Get_Device_IDA(
        parentDev, data->parent_deviceInstancePath, CAMERA_DATA_STRING_SIZE, 0);

    if (cmret != CR_SUCCESS) {
        log_info("CM_Get_Device_IDA parent fail: %s", data->deviceInstancePath);
        return;
    }

    ULONG szAddr;
    ULONG szDesc;

    szAddr = 4;
    szDesc = CAMERA_DATA_STRING_SIZE;
    cmret = CM_Get_DevNode_Registry_PropertyA(
        dnDevInst, CM_DRP_ADDRESS, NULL, &data->address, &szAddr, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA fail: %s",
            data->deviceInstancePath);
        return;
    }

    cmret = CM_Get_DevNode_Registry_PropertyA(
        dnDevInst, CM_DRP_DEVICEDESC, NULL, &data->name, &szDesc, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA fail: %s",
            data->deviceInstancePath);
        return;
    }

    szAddr = 4;
    szDesc = CAMERA_DATA_STRING_SIZE;
    cmret = CM_Get_DevNode_Registry_PropertyA(
        parentDev, CM_DRP_ADDRESS, NULL, &data->parent_address, &szAddr, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA parent fail: %s",
            data->deviceInstancePath);
        return;
    }

    cmret = CM_Get_DevNode_Registry_PropertyA(
        parentDev, CM_DRP_DEVICEDESC, NULL, &data->parent_name, &szDesc, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA parent fail: %s",
            data->deviceInstancePath);
        return;
    }

    log_info("Found %s @ %d", data->name, data->address);
    log_info("Parent %s @ %d", data->parent_name, data->parent_address);
    data->setup = true;
}

void camhook_init(struct camhook_config_cam *config_cam)
{
    MFStartup(0x20070u, 0);

    // fill before applying hooks
    for (size_t i = 0; i < config_cam->num_devices; ++i) {
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
    } else {
        log_info("No cams detected, not hooking");
    }
}

void camhook_fini(void)
{
    MFShutdown();
}

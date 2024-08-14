#define LOG_MODULE "cam-hook"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <initguid.h>
#include <winioctl.h>
#include <usbioctl.h>
// clang-format on

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>

#include <cfgmgr32.h>
#include <setupapi.h>

#include <stdio.h>

#include "camhook/cam.h"

#include "core/log.h"

#include "hook/com-proxy.h"
#include "hook/table.h"

#include "util/defs.h"
#include "util/str.h"

#define CAMHOOK_NUM_LAYOUTS 3

static struct camera_data camData[CAMHOOK_CONFIG_CAM_MAX];
int camAddresses[CAMHOOK_NUM_LAYOUTS][CAMHOOK_CONFIG_CAM_MAX] = {
    {1, 7},
    {4, 9},
    {4, 3},
};
static size_t num_addressed_cams = 0;
static size_t num_located_cams = 0;
static int camhook_port_layout = 0;

static enum camhook_version camhook_version = CAMHOOK_VERSION_OLD;

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

static BOOL STDCALL my_DeviceIoControl(
    HANDLE hFile,
    uint32_t dwIoControlCode,
    void *lpInBuffer,
    uint32_t nInBufferSize,
    void *lpOutBuffer,
    uint32_t nOutBufferSize,
    uint32_t *lpBytesReturned,
    OVERLAPPED *lpOverlapped);

static BOOL(STDCALL *real_DeviceIoControl)(
    HANDLE fd,
    uint32_t code,
    void *in_bytes,
    uint32_t in_nbytes,
    void *out_bytes,
    uint32_t out_nbytes,
    uint32_t *out_returned,
    OVERLAPPED *ovl);

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

static const struct hook_symbol camhook_cfgmgr32_syms_new[] = {
    {.name = "CM_Locate_DevNodeA",
     .patch = my_CM_Locate_DevNodeA,
     .link = (void **) &real_CM_Locate_DevNodeA},
};

static const struct hook_symbol camhook_mf_syms[] = {
    {.name = "MFEnumDeviceSources",
     .patch = my_MFEnumDeviceSources,
     .link = (void **) &real_MFEnumDeviceSources},
};

static struct hook_symbol camhook_ioctl_syms[] = {
    {.name = "DeviceIoControl",
     .patch = my_DeviceIoControl,
     .link = (void *) &real_DeviceIoControl},
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
                if (camhook_version == CAMHOOK_VERSION_OLD) {
                    snprintf(
                        builtString,
                        CAMERA_DATA_STRING_SIZE,
                        "USB\\VID_288C&PID_0002&MI_00\\%s",
                        camData[i].extra_upper);
                } else if (camhook_version == CAMHOOK_VERSION_NEW) {            
                    snprintf(
                        builtString,
                        CAMERA_DATA_STRING_SIZE,
                        "USB\\VID_05A3&PID_9230&MI_00\\%s",
                        camData[i].extra_upper);
                }

                log_info("built: %s", builtString);
                if (strcmp(pDeviceID, builtString) == 0) {
                    if (camhook_version == CAMHOOK_VERSION_OLD) {
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
                    } else if (camhook_version == CAMHOOK_VERSION_NEW) {
                        // inject original device
                        log_info(
                            "Injecting original device %d %s -> %s",
                            (int) i, pDeviceID, camData[i].deviceInstancePath);
                        pDeviceID = camData[i].deviceInstancePath;
                    }
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
    // log_info("Inside: %s", __FUNCTION__);

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
        if (camhook_version == CAMHOOK_VERSION_OLD) {
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
        } else if (camhook_version == CAMHOOK_VERSION_NEW) {
            // \\?\usb#vid_05a3&pid_9230&mi_00
            pwc[12] = L'0';
            pwc[13] = L'5';
            pwc[14] = L'a';
            pwc[15] = L'3';

            pwc[21] = L'9';
            pwc[22] = L'2';
            pwc[23] = L'3';
            pwc[24] = L'0';

            pwc[29] = L'0';
            pwc[30] = L'0';
        }

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

    // log_info("Inside: %s", __FUNCTION__);
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
                                // old style always uses set 0
                                camData[i].fake_address =
                                    camAddresses[0][num_addressed_cams];
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

ULONG get_matching_device_replacement_id(
    HANDLE hFile,
    ULONG ConnectionIndex,
    USB_DEVICE_DESCRIPTOR* desc)
{
    ULONG replacement_id = 0;
    for (size_t i = 0; i < CAMHOOK_CONFIG_CAM_MAX; ++i) {
        if (camData[i].setup) {
            // log_info("Checking %lu %04x %04x vs %lu %04x %04x",
            //     ConnectionIndex, desc->idVendor, desc->idProduct,
            //     camData[i].address, camData[i].vid, camData[i].pid);
            if (
                ConnectionIndex == camData[i].parent_address &&
                desc->idVendor == camData[i].vid &&
                desc->idProduct == camData[i].pid
            ) {
                // do secondary check for driver key using
                // IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME
                USB_NODE_CONNECTION_DRIVERKEY_NAME req;
                req.ActualLength = 0;
                req.ConnectionIndex = ConnectionIndex;
                req.DriverKeyName[0] = '\0';

                DWORD nBytes;

                if (!DeviceIoControl(
                        hFile,
                        IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                        &req,
                        sizeof(req),
                        &req,
                        sizeof(req),
                        &nBytes,
                        0LL
                )) {
                    log_warning(
                        "Failed to get driver key name size for device %04x %04x",
                        desc->idVendor,
                        desc->idProduct);
                    continue;
                }

                if (req.ActualLength <= sizeof(req)) {
                    log_warning(
                        "Driver key name size too small for device %04x %04x",
                        desc->idVendor,
                        desc->idProduct);
                    continue;
                }

                nBytes = req.ActualLength;

                USB_NODE_CONNECTION_DRIVERKEY_NAME *driverKeyNameW = malloc(nBytes);
                if (!driverKeyNameW) {
                    log_warning("Failed to allocate driver key name buffer for device %04x %04x",
                        desc->idVendor,
                        desc->idProduct);
                    continue;
                }
                driverKeyNameW->ActualLength = 0;
                driverKeyNameW->ConnectionIndex = ConnectionIndex;
                driverKeyNameW->DriverKeyName[0] = '\0';

                if (!DeviceIoControl(
                        hFile,
                        IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                        driverKeyNameW,
                        nBytes,
                        driverKeyNameW,
                        nBytes,
                        &nBytes,
                        0LL
                )) {
                    log_warning(
                        "Failed to get driver key name for device %04x %04x",
                        desc->idVendor,
                        desc->idProduct);
                    continue;
                }

                
                char *driverKeyNameA = NULL;
                wstr_narrow(driverKeyNameW->DriverKeyName, &driverKeyNameA);

                free(driverKeyNameW);


                if (strcmp(driverKeyNameA, camData[i].parent_driverKey) != 0) {
                    // log just in case?
                    log_info(
                        "Driver key name mismatch for device %04x %04x %s != %s",
                        desc->idVendor,
                        desc->idProduct,
                        driverKeyNameA,
                        camData[i].parent_driverKey);
                    if (driverKeyNameA) {
                        free(driverKeyNameA);
                    }
                    continue;
                }

                log_info(
                    "Replacing device @ %lu with %d for %04x %04x %s",
                    ConnectionIndex,
                    camAddresses[camhook_port_layout][i],
                    desc->idVendor,
                    desc->idProduct,
                    driverKeyNameA);

                if (driverKeyNameA) {
                    free(driverKeyNameA);
                }

                replacement_id = camAddresses[camhook_port_layout][i];
                break;
            }
        }
    }

    return replacement_id;
}

static BOOL STDCALL my_DeviceIoControl(
    HANDLE hFile,
    uint32_t dwIoControlCode,
    void *lpInBuffer,
    uint32_t nInBufferSize,
    void *lpOutBuffer,
    uint32_t nOutBufferSize,
    uint32_t *lpBytesReturned,
    OVERLAPPED *lpOverlapped)
{
    BOOL res;

    res = real_DeviceIoControl(
        hFile,
        dwIoControlCode,
        lpInBuffer,
        nInBufferSize,
        lpOutBuffer,
        nOutBufferSize,
        lpBytesReturned,
        lpOverlapped);

    // if error just return
    if (!res) {
        return res;
    }

    // detect IOCTL_USB_GET_NODE_CONNECTION_INFORMATION (_EX)
    // we don't bother faking the rest as it's never read
    if (dwIoControlCode == IOCTL_USB_GET_NODE_CONNECTION_INFORMATION) {
        USB_NODE_CONNECTION_INFORMATION *connectionInfo = lpOutBuffer;

        ULONG replacement_id = get_matching_device_replacement_id(
            hFile, connectionInfo->ConnectionIndex, &connectionInfo->DeviceDescriptor);
        if (replacement_id > 0) {
            connectionInfo->ConnectionIndex = replacement_id;
        }
    } else if (dwIoControlCode == IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX) {
        USB_NODE_CONNECTION_INFORMATION_EX *connectionInfoEx = lpOutBuffer;

        ULONG replacement_id = get_matching_device_replacement_id(
            hFile, connectionInfoEx->ConnectionIndex, &connectionInfoEx->DeviceDescriptor);
        if (replacement_id > 0) {
            connectionInfoEx->ConnectionIndex = replacement_id;
        }
    }

    return res;
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

    camhook_port_layout = config_cam->port_layout;
    if (camhook_port_layout < 0 || camhook_port_layout >= CAMHOOK_NUM_LAYOUTS) {
        camhook_port_layout = 0;
    }

    if (num_setup > 0) {
        // always
        hook_table_apply(
            NULL, "Mf.dll", camhook_mf_syms, lengthof(camhook_mf_syms));

        if (camhook_version == CAMHOOK_VERSION_OLD) {
            hook_table_apply(
                NULL,
                "cfgmgr32.dll",
                camhook_cfgmgr32_syms,
                lengthof(camhook_cfgmgr32_syms));
        } else if (camhook_version == CAMHOOK_VERSION_NEW) {
            // for CAMHOOK_VERSION_NEW we restore original VID/PID
            // so the parent lookup succeeds later
            // yes the DLL moved???
            hook_table_apply(
                NULL,
                "setupapi.dll",
                camhook_cfgmgr32_syms_new,
                lengthof(camhook_cfgmgr32_syms_new));

            // they copied / forked usb/usbview/enum.c
            // however they don't use most of it
            // so we only need DeviceIoControl to inject port #
            hook_table_apply(
                NULL,
                "kernel32.dll",
                camhook_ioctl_syms,
                lengthof(camhook_ioctl_syms));
        }

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

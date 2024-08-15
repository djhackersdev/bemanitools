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

#include "camhook/cam-detect.h"

#include "iface-core/log.h"

#include "util/defs.h"
#include "util/str.h"

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

void fill_cam_struct(struct camera_data *data, const char *devid)
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

    char* vid_str = strstr(data->deviceInstancePath, "VID_");
    char* pid_str = strstr(data->deviceInstancePath, "PID_");

    if (!vid_str || !pid_str) {
        log_info("Could not parse VID/PID in %s", data->deviceInstancePath);
        return;
    }

    data->vid = strtol(vid_str + 4, NULL, 16);
    data->pid = strtol(pid_str + 4, NULL, 16);

    log_info("vid: %04x", data->vid);
    log_info("pid: %04x", data->pid);


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
    ULONG szDriverKey;

    szAddr = 4;
    cmret = CM_Get_DevNode_Registry_PropertyA(
        dnDevInst, CM_DRP_ADDRESS, NULL, &data->address, &szAddr, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA CM_DRP_ADDRESS fail: %s",
            data->deviceInstancePath);
        return;
    }

    szDesc = CAMERA_DATA_STRING_SIZE;
    cmret = CM_Get_DevNode_Registry_PropertyA(
        dnDevInst, CM_DRP_DEVICEDESC, NULL, &data->name, &szDesc, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA CM_DRP_DEVICEDESC fail: %s",
            data->deviceInstancePath);
        return;
    }

    szAddr = 4;
    cmret = CM_Get_DevNode_Registry_PropertyA(
        parentDev, CM_DRP_ADDRESS, NULL, &data->parent_address, &szAddr, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA CM_DRP_ADDRESS parent fail: %s",
            data->deviceInstancePath);
        return;
    }

    szDesc = CAMERA_DATA_STRING_SIZE;
    cmret = CM_Get_DevNode_Registry_PropertyA(
        parentDev, CM_DRP_DEVICEDESC, NULL, &data->parent_name, &szDesc, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA CM_DRP_DEVICEDESC parent fail: %s",
            data->deviceInstancePath);
        return;
    }

    szDriverKey = CAMERA_DATA_STRING_SIZE;
    cmret = CM_Get_DevNode_Registry_PropertyA(
        parentDev, CM_DRP_DRIVER, NULL, &data->parent_driverKey, &szDriverKey, 0);

    if (cmret != CR_SUCCESS) {
        log_info(
            "CM_Get_DevNode_Registry_PropertyA CM_DRP_DRIVER parent fail: %s",
            data->deviceInstancePath);
        return;
    }

    log_info("Found %s @ %d", data->name, data->address);
    log_info("Parent %s @ %d %s", data->parent_name, data->parent_address, data->parent_driverKey);
    data->setup = true;
}

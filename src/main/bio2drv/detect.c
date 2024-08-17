#define LOG_MODULE "bio2drv-detect"

#include <initguid.h>
#include <windows.h>

#include <cfgmgr32.h>
#include <setupapi.h>

#include <stdio.h>
#include <string.h>

#include "bio2drv/detect.h"

#include "iface-core/log.h"

DEFINE_GUID(
    GUID_COM_BUS_ENUMERATOR,
    0x4D36E978,
    0xE325,
    0x11CE,
    0xBF,
    0xC1,
    0x08,
    0x00,
    0x2B,
    0xE1,
    0x03,
    0x18);

static char work_buffer[0x400];

static bool check_property(
    HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA PDeviceInfoData, DWORD property)
{
    if (SetupDiGetDeviceRegistryPropertyA(
            DeviceInfoSet,
            PDeviceInfoData,
            property,
            NULL,
            (BYTE *) work_buffer,
            sizeof(work_buffer),
            NULL)) {
        if (strstr(work_buffer, "BIO2(VIDEO)")) {
            log_info("Found matching device: %s", work_buffer);
            return true;
        }
    }

    log_info("Device with property: %s does not match", work_buffer);
    return false;
}

static bool check_id(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA PDeviceInfoData)
{
    if (CM_Get_Device_IDA(
            PDeviceInfoData->DevInst, work_buffer, sizeof(work_buffer), 0)) {
        return false;
    }

    if (strstr(work_buffer, "VID_1CCF&PID_804C")) {
        log_info("Found matching device: %s", work_buffer);
        return true;
    }

    if (strstr(work_buffer, "VID_1CCF&PID_8040")) {
        log_info("Found matching device: %s", work_buffer);
        return true;
    }

    log_info("Device with ID: %s does not match", work_buffer);
    return false;
}

static bool get_device_by_filter(
    bool id_filter, DWORD property, size_t devnum, char *path, size_t length)
{
    HDEVINFO DeviceInfoSet = SetupDiGetClassDevsA(
        &GUID_COM_BUS_ENUMERATOR, NULL, NULL, DIGCF_PRESENT);

    if (DeviceInfoSet == INVALID_HANDLE_VALUE) {
        return false;
    }

    SP_DEVINFO_DATA DeviceInfoData;
    ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    DWORD idx = 0;
    size_t num_found = 0;

    while (SetupDiEnumDeviceInfo(DeviceInfoSet, idx, &DeviceInfoData)) {
        HKEY DeviceRegKey = SetupDiOpenDevRegKey(
            DeviceInfoSet,
            &DeviceInfoData,
            DICS_FLAG_GLOBAL,
            0,
            DIREG_DEV,
            KEY_QUERY_VALUE);

        if (DeviceRegKey) {
            bool found = false;
            log_info("Found a serial device at index: %ld", idx);

            if (!id_filter) {
                if (check_property(DeviceInfoSet, &DeviceInfoData, property)) {
                    found = true;
                }
            } else {
                if (check_id(DeviceInfoSet, &DeviceInfoData)) {
                    found = true;
                }
            }

            if (found) {
                if (num_found == devnum) {
                    DWORD path_length = length;
                    RegQueryValueExA(
                        DeviceRegKey,
                        "PortName",
                        0,
                        NULL,
                        (BYTE *) path,
                        &path_length);
                    log_info("Using port: %s", path);

                    return true;
                }
                num_found++;
            }
        }
        ++idx;
    }
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);

    log_warning("No matching device found");
    return false;
}

bool bio2drv_detect(
    enum bio2drv_detect_mode mode, size_t index, char *path, size_t length)
{
    if (mode == DETECT_DEVICEDESC) {
        return get_device_by_filter(
            false, SPDRP_DEVICEDESC, index, path, length);
    } else if (mode == DETECT_FRIENDLYNAME) {
        return get_device_by_filter(
            false, SPDRP_FRIENDLYNAME, index, path, length);
    } else if (mode == DETECT_DEVICEID) {
        return get_device_by_filter(true, -1, index, path, length);
    }

    log_warning("Unknown autodetect mode: %d", mode);
    return false;
}

#ifndef NVGPU_CONFIG_NVAPI_H
#define NVGPU_CONFIG_NVAPI_H

#include <stdint.h>

// Remark: nvapi is not compatible with mingw and requires some changes, see
// https://forums.developer.nvidia.com/t/how-can-you-compile-nvapi-h-with-mingw/33050/5
#include "imports/nvapi/nvapi.h"
#include "imports/nvapi/NvApiDriverSettings.h"

typedef NvAPI_Status (*NvAPI_Initialize_t)();
typedef NvAPI_Status (*NvAPI_Unload_t)();
typedef NvAPI_Status (*NvAPI_GetInterfaceVersionStringEx_t)(NvAPI_ShortString);
typedef NvAPI_Status (*NvAPI_SYS_GetDriverAndBranchVersion_t)(NvU32 *, NvAPI_ShortString);
typedef NvAPI_Status (*NvAPI_GetErrorMessage_t)(NvAPI_Status, NvAPI_ShortString);
typedef NvAPI_Status (*NvAPI_DRS_CreateSession_t)(NvDRSSessionHandle *);
typedef NvAPI_Status (*NvAPI_DRS_LoadSettings_t)(NvDRSSessionHandle);
typedef NvAPI_Status (*NvAPI_DRS_FindProfileByName_t)(NvDRSSessionHandle, NvAPI_UnicodeString, NvDRSProfileHandle *);
typedef NvAPI_Status (*NvAPI_DRS_FindApplicationByName_t)(NvDRSSessionHandle, NvAPI_UnicodeString, NvDRSProfileHandle *, NVDRS_APPLICATION *);
typedef NvAPI_Status (*NvAPI_DRS_CreateProfile_t)(NvDRSSessionHandle, NVDRS_PROFILE *, NvDRSProfileHandle *);
typedef NvAPI_Status (*NvAPI_DRS_CreateApplication_t)(NvDRSSessionHandle, NvDRSProfileHandle, NVDRS_APPLICATION *);
typedef NvAPI_Status (*NvAPI_DRS_GetSetting_t)(NvDRSSessionHandle, NvDRSProfileHandle, NvU32, NVDRS_SETTING *);
typedef NvAPI_Status (*NvAPI_DRS_SetSetting_t)(NvDRSSessionHandle, NvDRSProfileHandle, NVDRS_SETTING *);
typedef NvAPI_Status (*NvAPI_DRS_SaveSettings_t)(NvDRSSessionHandle);
typedef NvAPI_Status (*NvAPI_DRS_DestroySession_t)(NvDRSSessionHandle);
typedef NvAPI_Status (*NvAPI_EnumPhysicalGPUs_t)(NvPhysicalGpuHandle *, NvU32 *);
typedef NvAPI_Status (*NvAPI_GPU_GetConnectedDisplayIds_t)(NvPhysicalGpuHandle, NV_GPU_DISPLAYIDS*, NvU32 *, NvU32);
typedef NvAPI_Status (*NvAPI_DISP_GetTiming_t)(NvU32, NV_TIMING_INPUT*, NV_TIMING*);
typedef NvAPI_Status (*NvAPI_DISP_TryCustomDisplay_t)(NvU32 *, NvU32, NV_CUSTOM_DISPLAY *);
typedef NvAPI_Status (*NvAPI_DISP_SaveCustomDisplay_t)(NvU32*, NvU32, NvU32, NvU32);
typedef NvAPI_Status (*NvAPI_DISP_RevertCustomDisplayTrial_t)(NvU32*, NvU32);
typedef NvAPI_Status (*NvAPI_DRS_DeleteProfile_t)(NvDRSSessionHandle, NvDRSProfileHandle);
typedef NvAPI_Status (*NvAPI_DISP_GetDisplayConfig_t)(NvU32*, NV_DISPLAYCONFIG_PATH_INFO*);
typedef NvAPI_Status (*NvAPI_DISP_SetDisplayConfig_t)(NvU32, NV_DISPLAYCONFIG_PATH_INFO*, NvU32);

typedef struct nv_api {
    NvAPI_Initialize_t NvAPI_Initialize;
    NvAPI_Unload_t NvAPI_Unload;
    NvAPI_GetInterfaceVersionStringEx_t NvAPI_GetInterfaceVersionStringEx;
    NvAPI_SYS_GetDriverAndBranchVersion_t NvAPI_SYS_GetDriverAndBranchVersion;
    NvAPI_GetErrorMessage_t NvAPI_GetErrorMessage;
    NvAPI_DRS_CreateSession_t NvAPI_DRS_CreateSession;
    NvAPI_DRS_LoadSettings_t NvAPI_DRS_LoadSettings;
    NvAPI_DRS_FindProfileByName_t NvAPI_DRS_FindProfileByName;
    NvAPI_DRS_FindApplicationByName_t NvAPI_DRS_FindApplicationByName;
    NvAPI_DRS_CreateProfile_t NvAPI_DRS_CreateProfile;
    NvAPI_DRS_CreateApplication_t NvAPI_DRS_CreateApplication;
    NvAPI_DRS_GetSetting_t NvAPI_DRS_GetSetting;
    NvAPI_DRS_SetSetting_t NvAPI_DRS_SetSetting;
    NvAPI_DRS_SaveSettings_t NvAPI_DRS_SaveSettings;
    NvAPI_DRS_DestroySession_t NvAPI_DRS_DestroySession;
    NvAPI_EnumPhysicalGPUs_t NvAPI_EnumPhysicalGPUs;
    NvAPI_GPU_GetConnectedDisplayIds_t NvAPI_GPU_GetConnectedDisplayIds;
    NvAPI_DISP_GetTiming_t NvAPI_DISP_GetTiming;
    NvAPI_DISP_TryCustomDisplay_t NvAPI_DISP_TryCustomDisplay;
    NvAPI_DISP_SaveCustomDisplay_t NvAPI_DISP_SaveCustomDisplay;
    NvAPI_DISP_RevertCustomDisplayTrial_t NvAPI_DISP_RevertCustomDisplayTrial;
    NvAPI_DRS_DeleteProfile_t NvAPI_DRS_DeleteProfile;
    NvAPI_DISP_GetDisplayConfig_t NvAPI_DISP_GetDisplayConfig;
    NvAPI_DISP_SetDisplayConfig_t NvAPI_DISP_SetDisplayConfig;
} nv_api_t;

#endif
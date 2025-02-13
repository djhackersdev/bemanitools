#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "nv/api.h"
#include "nv/module.h"

#include "util/fs.h"
#include "util/mem.h"
#include "util/str.h"

#define NV_DRS_SETTINGS_FOLDER "C:\\ProgramData\\NVIDIA Corporation\\Drs"

#define printf_out(fmt, ...) \
    fprintf(stdout, fmt, ##__VA_ARGS__)
#define printf_err(fmt, ...) \
    fprintf(stderr, fmt, ##__VA_ARGS__)
#define printfln_out(fmt, ...) \
    fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#define printfln_err(fmt, ...) \
    fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#define PRINT_ERR_WITH_NVAPI_MESSAGE(status, fmt, ...) \
    NvAPI_ShortString error_str; \
    nv_api->NvAPI_GetErrorMessage(status, error_str); \
    fprintf(stderr, fmt ", reason: %s\n", ##__VA_ARGS__, error_str);

typedef struct displayconfig_path_info {
    NvU32 path_info_count;
    NV_DISPLAYCONFIG_PATH_INFO *path_info;
} displayconfig_path_info_t;

static bool _session_destroy(const nv_api_t *nv_api, NvDRSSessionHandle session)
{
    assert(nv_api);
    assert(session);

    NvAPI_Status status;

    status = nv_api->NvAPI_DRS_DestroySession(session);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Destroying driver settings session");
        return false;
    }

    return true;
}

static bool _session_create_and_settings_load(const nv_api_t *nv_api, NvDRSSessionHandle *session)
{
    assert(nv_api);
    assert(session);

    NvAPI_Status status;

    status = nv_api->NvAPI_DRS_CreateSession(session);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Creating driver settings session");
        return false;
    }

    status = nv_api->NvAPI_DRS_LoadSettings(*session);
    
    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Loading driver settings");
        
        return _session_destroy(nv_api, *session);
    }

    return true;
}

static bool _settings_save_and_session_destroy(const nv_api_t *nv_api, NvDRSSessionHandle session)
{
    assert(nv_api);
    assert(session);

    NvAPI_Status status;

    printfln_err("Saving ...");

    status = nv_api->NvAPI_DRS_SaveSettings(session);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Saving driver settings");
    
        return _session_destroy(nv_api, session);
    }

    return _session_destroy(nv_api, session);
}

static bool _profile_get(
        const nv_api_t *nv_api,
        const char *profile_name,
        NvDRSSessionHandle session,
        NvDRSProfileHandle *profile)
{
    NvAPI_UnicodeString nv_profile_name;
    NvAPI_Status status;
    wchar_t *profile_name_wstr;

    assert(nv_api);
    assert(session);
    assert(profile);

    profile_name_wstr = str_widen(profile_name);
    wstr_cpy(nv_profile_name, sizeof(nv_profile_name), profile_name_wstr);

    status = nv_api->NvAPI_DRS_FindProfileByName(session, nv_profile_name, profile);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Finding driver profile %s", profile_name);
        return false;
    }

    return true;
}

static bool _profile_setting_set(
    const nv_api_t *nv_api,
    NvDRSSessionHandle session,
    NvDRSProfileHandle profile,
    NVDRS_SETTING *setting)
{
    NvAPI_Status status;

    assert(nv_api);
    assert(setting);

    status = nv_api->NvAPI_DRS_SetSetting(session, profile, setting);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "Error setting driver settings");
        return false;
    }

    return true;
}

static void _display_config_free(displayconfig_path_info_t *displayconfig_path_info)
{
    assert(displayconfig_path_info);
    assert(displayconfig_path_info->path_info);

    for (NvU32 i = 0; i < displayconfig_path_info->path_info_count; i++) {
        assert(displayconfig_path_info->path_info[i].sourceModeInfo);
        assert(displayconfig_path_info->path_info[i].targetInfo);

        for (NvU32 j = 0; j < displayconfig_path_info->path_info[i].targetInfoCount; j++) {
            assert(displayconfig_path_info->path_info[i].targetInfo[i].details);

            free(displayconfig_path_info->path_info[i].targetInfo[i].details);
        }

        free(displayconfig_path_info->path_info[i].targetInfo);
        free(displayconfig_path_info->path_info[i].sourceModeInfo);
    }

    free(displayconfig_path_info->path_info);
}

static bool _display_config_get(const nv_api_t *nv_api, displayconfig_path_info_t *displayconfig_path_info)
{
    NvAPI_Status status;

    assert(nv_api);
    assert(displayconfig_path_info);

    // TODO we need a total of three calls to GetDisplayConfig: https://github.com/NVIDIA/nvapi/blob/d08488fcc82eef313b0464db37d2955709691e94/Sample_Code/DisplayConfiguration/DisplayConfiguration.cpp#L68

    status = nv_api->NvAPI_DISP_GetDisplayConfig(&displayconfig_path_info->path_info_count , NULL);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting display config path info count");
        return false;
    }

    displayconfig_path_info->path_info = (NV_DISPLAYCONFIG_PATH_INFO*)
        xmalloc(sizeof(NV_DISPLAYCONFIG_PATH_INFO) * displayconfig_path_info->path_info_count);
    memset(displayconfig_path_info->path_info, 0, sizeof(NV_DISPLAYCONFIG_PATH_INFO) * displayconfig_path_info->path_info_count);

    for (NvU32 i = 0; i < displayconfig_path_info->path_info_count; i++) {
        displayconfig_path_info->path_info[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER;
    }

    status = nv_api->NvAPI_DISP_GetDisplayConfig(&displayconfig_path_info->path_info_count, displayconfig_path_info->path_info);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting display config target info counts");

        free(displayconfig_path_info->path_info);

        return false;
    }

    for (NvU32 i = 0; i < displayconfig_path_info->path_info_count; i++) {
        if (displayconfig_path_info->path_info[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER1 ||
                displayconfig_path_info->path_info[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER2) {
            displayconfig_path_info->path_info[i].sourceModeInfo =
                (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*) xmalloc(sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
        } else {
#ifdef NV_DISPLAYCONFIG_PATH_INFO_VER3
            displayconfig_path_info->path_info[i].sourceModeInfo =(NV_DISPLAYCONFIG_SOURCE_MODE_INFO*) malloc(
                displayconfig_path_info->path_info[i].sourceModeInfoCount * sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
#endif
        }

        memset(displayconfig_path_info->path_info[i].sourceModeInfo, 0, sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));

        displayconfig_path_info->path_info[i].targetInfo = (NV_DISPLAYCONFIG_PATH_TARGET_INFO*) xmalloc(
            displayconfig_path_info->path_info[i].targetInfoCount * sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
   
        memset(displayconfig_path_info->path_info[i].targetInfo, 0,
            displayconfig_path_info->path_info[i].targetInfoCount * sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
        
        for (NvU32 j = 0 ; j < displayconfig_path_info->path_info[i].targetInfoCount ; j++) {
            displayconfig_path_info->path_info[i].targetInfo[j].details =
                (NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO*) xmalloc(sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));    
            memset(displayconfig_path_info->path_info[i].targetInfo[j].details, 0,
                sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
            displayconfig_path_info->path_info[i].targetInfo[j].details->version =
                NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER;
        }
    }

    status = nv_api->NvAPI_DISP_GetDisplayConfig(&displayconfig_path_info->path_info_count, displayconfig_path_info->path_info);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting display config");

        _display_config_free(displayconfig_path_info);

        return false;
    }

    return true;
}

static bool _display_config_set(const nv_api_t *nv_api, displayconfig_path_info_t *displayconfig_path_info)
{
    NvAPI_Status status;

    assert(nv_api);
    assert(displayconfig_path_info);

    printfln_err("Validating display config...");

    status = nv_api->NvAPI_DISP_SetDisplayConfig(displayconfig_path_info->path_info_count, displayconfig_path_info->path_info, NV_DISPLAYCONFIG_VALIDATE_ONLY);
    
    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Validating display config");
        return false;
    }

    printfln_err("Saving display config...");

    status = nv_api->NvAPI_DISP_SetDisplayConfig(displayconfig_path_info->path_info_count, displayconfig_path_info->path_info, NV_DISPLAYCONFIG_SAVE_TO_PERSISTENCE | NV_DISPLAYCONFIG_DRIVER_RELOAD_ALLOWED);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Saving display config");
        return false;
    }

    return true;
}

static bool _display_config_visit(displayconfig_path_info_t *displayconfig_path_info, NvU32 display_id, bool (*display_config_filter)(NV_DISPLAYCONFIG_PATH_TARGET_INFO *target_info))
{
    NV_DISPLAYCONFIG_PATH_TARGET_INFO *target_info;

    for (NvU32 i = 0; i < displayconfig_path_info->path_info_count; i++) {
        target_info = &displayconfig_path_info->path_info[i].targetInfo[i];

        if (display_id != 0 && target_info->displayId != display_id) {
            continue;
        }

        return display_config_filter(target_info);
    }

    return false;
}

static void _ensure_drs_settings_folder_exists()
{
    // Even on a fresh install, this might not exist which result
    // in NVAPI_ACCESS_DENIED errors on many (or even all?) nvapi DRS functions
    if (!path_exists(NV_DRS_SETTINGS_FOLDER)) {
        printfln_err("NOTE: DRS settings folder to store profiles does not exist, creating...");
        path_mkdir(NV_DRS_SETTINGS_FOLDER);
    }
}

// -------------------------------------------------------------------------------------------------

static bool _nv_info(const nv_api_t *nv_api)
{
    NvAPI_ShortString interface_version;
    NvAPI_ShortString driver_version;
    NvU32 driver_version_number;
    NvAPI_Status status;

    printfln_err("Getting interface version...");

    status = nv_api->NvAPI_GetInterfaceVersionStringEx(interface_version);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting interface version string");
        return false;
    }

    printfln_err("Getting driver and branch version...");

    status = nv_api->NvAPI_SYS_GetDriverAndBranchVersion(&driver_version_number, driver_version);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting driver and branch version");
        return false;
    }

    printfln_out("Interface version: %s", interface_version);
    printfln_out("Driver version: %s", driver_version);
    printfln_out("Driver version number: %lu", driver_version_number);

    return true;
}

static bool _profile_create(const nv_api_t *nv_api, const char *profile_name)
{
    NvDRSSessionHandle session;
    NVDRS_PROFILE profile;
    wchar_t *profile_name_wstr;
    NvAPI_Status status;
    NvDRSProfileHandle profile_handle;

    assert(nv_api);
    assert(profile_name);

    if (!_session_create_and_settings_load(nv_api, &session)) {
        return false;
    }

    profile.version = NVDRS_PROFILE_VER;

    profile_name_wstr = str_widen(profile_name);
    wstr_cpy(profile.profileName, sizeof(profile.profileName), profile_name_wstr);
    free(profile_name_wstr);

    printfln_err("Creating profile %s...", profile_name);

    status = nv_api->NvAPI_DRS_CreateProfile(session, &profile, &profile_handle);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Creating driver profile");
        
        _session_destroy(nv_api, session);
        return false;
    }

    return _settings_save_and_session_destroy(nv_api, session);
}

static bool _profile_delete(const nv_api_t *nv_api, const char *profile_name)
{
    NvDRSSessionHandle session;
    NvDRSProfileHandle profile;
    NvAPI_Status status;

    assert(nv_api);
    assert(profile_name);

    if (!_session_create_and_settings_load(nv_api, &session)) {
        return false;
    }

    if (!_profile_get(nv_api, profile_name, session, &profile)) {
        _session_destroy(nv_api, session);

        return false;
    }

    printfln_err("Deleting profile %s...", profile_name);

    status = nv_api->NvAPI_DRS_DeleteProfile(session, profile);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Deleting driver profile %s", profile_name);

        _session_destroy(nv_api, session);

        return false;
    }

    return _settings_save_and_session_destroy(nv_api, session);
}

static bool _profile_application_add(const nv_api_t *nv_api, const char *profile_name, const char *application_name)
{
    NvDRSSessionHandle session;
    NvDRSProfileHandle profile;
    NvAPI_Status status;
    NVDRS_APPLICATION application;
    wchar_t *application_name_wstr;

    assert(nv_api);
    assert(profile_name);
    assert(application_name);

    if (!_session_create_and_settings_load(nv_api, &session)) {
        return false;
    }

    if (!_profile_get(nv_api, profile_name, session, &profile)) {
        _session_destroy(nv_api, session);

        return false;
    }

    application.version = NVDRS_APPLICATION_VER;

    application_name_wstr = str_widen(application_name);
    wstr_cpy(application.appName, sizeof(application.appName), application_name_wstr);
    free(application_name_wstr);

    printfln_err("Adding application %s to profile %s...", application_name, profile_name);

    status = nv_api->NvAPI_DRS_CreateApplication(session, profile, &application);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Adding application %s to profile %s", application_name, profile_name);

        _session_destroy(nv_api, session);

        return false;
    }

    return _settings_save_and_session_destroy(nv_api, session);
}

static bool _profile_gsync_disable(const nv_api_t *nv_api, const char *profile_name)
{
    NvDRSSessionHandle session;
    NvDRSProfileHandle profile;
    NVDRS_SETTING setting;

    assert(nv_api);
    assert(profile_name);

    if (!_session_create_and_settings_load(nv_api, &session)) {
        return false;
    }

    if (!_profile_get(nv_api, profile_name, session, &profile)) {
        _session_destroy(nv_api, session);

        return false;
    }
    
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = VRR_APP_OVERRIDE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32PredefinedValue = VRR_APP_OVERRIDE_FIXED_REFRESH;
    setting.u32CurrentValue = VRR_APP_OVERRIDE_FIXED_REFRESH;

    printfln_err("Disabling G-SYNC for profile %s...", profile_name);

    if (!_profile_setting_set(nv_api, session, profile, &setting)) {
        _session_destroy(nv_api, session);

        return false;
    }

    return _settings_save_and_session_destroy(nv_api, session);
}

static bool _profile_gpu_power_state_max(const nv_api_t *nv_api, const char *profile_name)
{
    NvDRSSessionHandle session;
    NvDRSProfileHandle profile;
    NVDRS_SETTING setting;

    assert(nv_api);
    assert(profile_name);

    if (!_session_create_and_settings_load(nv_api, &session)) {
        return false;
    }

    if (!_profile_get(nv_api, profile_name, session, &profile)) {
        _session_destroy(nv_api, session);

        return false;
    }

    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PREFERRED_PSTATE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.u32PredefinedValue = PREFERRED_PSTATE_PREFER_MAX;
    setting.u32CurrentValue = PREFERRED_PSTATE_PREFER_MAX;

    printfln_err("Setting GPU power state to maximum for profile %s...", profile_name);

    if (!_profile_setting_set(nv_api, session, profile, &setting)) {
        _session_destroy(nv_api, session);  

        return false;
    }

    return _settings_save_and_session_destroy(nv_api, session);
}

static bool _display_primary_display_id(const nv_api_t *nv_api)
{
    NvAPI_Status status;
    NvPhysicalGpuHandle gpu_handle[NVAPI_MAX_PHYSICAL_GPUS];
    NvU32 gpu_count;
    NvU32 display_id_count;
    NV_GPU_DISPLAYIDS *display_ids;
 
    assert(nv_api);
    
    status = nv_api->NvAPI_EnumPhysicalGPUs(gpu_handle, &gpu_count);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Enumerating physical GPUs");
        return false;
    }

    // Assuming first physical GPU, first display = primary display
    if (gpu_count > 0) {
        status = nv_api->NvAPI_GPU_GetConnectedDisplayIds(gpu_handle[0], NULL, &display_id_count, 0);

        if (status != NVAPI_OK) {
            PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting connected display count of GPU");
            return false;
        }

        if (display_id_count > 0) {
            display_ids = (NV_GPU_DISPLAYIDS*) xmalloc(sizeof(NV_GPU_DISPLAYIDS) * display_id_count);
            
            for (NvU32 j = 0; j < display_id_count; j++) {
                display_ids[j].version = NV_GPU_DISPLAYIDS_VER;
            }

            status = nv_api->NvAPI_GPU_GetConnectedDisplayIds(gpu_handle[0], display_ids, &display_id_count, 0);

            if (status != NVAPI_OK) {
                free(display_ids);
                PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting connected display ids of GPU");
                return false;
            }

            printfln_out("0x%lX", display_ids[0].displayId);

            free(display_ids);

            return true;
        }
    }

    return false;
}

static bool _displays_list(const nv_api_t *nv_api)
{
    NvAPI_Status status;
    NvPhysicalGpuHandle gpu_handle[NVAPI_MAX_PHYSICAL_GPUS];
    NvU32 gpu_count;
    NvU32 display_id_count;
    NV_GPU_DISPLAYIDS *display_ids;
    const char *connector_type;

    assert(nv_api);
    
    status = nv_api->NvAPI_EnumPhysicalGPUs(gpu_handle, &gpu_count);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Enumerating physical GPUs");
        return false;
    }

    printfln_err("Display ID, Connector Type, Active, Connected, Physically Connected");

    for (NvU32 i = 0; i < gpu_count; i++) {
        // Get display count per GPU
        display_id_count = 0;
        status = nv_api->NvAPI_GPU_GetConnectedDisplayIds(gpu_handle[i], NULL, &display_id_count, 0);

        if (status != NVAPI_OK) {
            PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting connected display count of GPU");
            return false;
        }

        if (display_id_count > 0) {
            display_ids = (NV_GPU_DISPLAYIDS*) xmalloc(sizeof(NV_GPU_DISPLAYIDS) * display_id_count);
            
            for (NvU32 j = 0; j < display_id_count; j++) {
                display_ids[j].version = NV_GPU_DISPLAYIDS_VER;
            }

            status = nv_api->NvAPI_GPU_GetConnectedDisplayIds(gpu_handle[i], display_ids, &display_id_count, 0);

            if (status != NVAPI_OK) {
                free(display_ids);
                PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Getting connected display ids of GPU");
                return false;
            }

            for (NvU32 j = 0; j < display_id_count; j++) {
                switch (display_ids[j].connectorType) {
                    case NV_MONITOR_CONN_TYPE_VGA:
                        connector_type = "VGA";
                        break;
                    case NV_MONITOR_CONN_TYPE_COMPONENT:
                        connector_type = "Component";
                        break;
                    case NV_MONITOR_CONN_TYPE_SVIDEO:
                        connector_type = "S-Video";
                        break;
                    case NV_MONITOR_CONN_TYPE_HDMI:
                        connector_type = "HDMI";
                        break;
                    case NV_MONITOR_CONN_TYPE_DVI:
                        connector_type = "DVI";
                        break;
                    case NV_MONITOR_CONN_TYPE_LVDS:
                        connector_type = "LVDS";
                        break;
                    case NV_MONITOR_CONN_TYPE_DP:
                        connector_type = "DP";
                        break;
                    case NV_MONITOR_CONN_TYPE_COMPOSITE:
                        connector_type = "Composite";
                        break;
                    default:
                        connector_type = "unknown";
                        break;
                }

                printfln_out("0x%lX, %s, %s, %s, %s", 
                    display_ids[j].displayId,
                    connector_type,
                    display_ids[j].isActive ? "active" : "inactive",
                    display_ids[j].isConnected ? "connected" : "disconnected",
                    display_ids[j].isPhysicallyConnected ? "connected" : "disconnected");
            }

            free(display_ids);
        }
    }

    return true;    
}

static bool _display_config_print(const nv_api_t *nv_api, NvU32 display_id)
{
    displayconfig_path_info_t displayconfig_path_info;
    NV_DISPLAYCONFIG_PATH_TARGET_INFO_V2 *target_info;
    NV_DISPLAYCONFIG_SOURCE_MODE_INFO_V1 *source_mode_info;

    assert(nv_api);

    _display_config_get(nv_api, &displayconfig_path_info);

    if (display_id != 0) {
        printfln_err("Applying display ID filter: %lX", display_id);
    }

    printfln_err("Num total adapters in config: %ld", displayconfig_path_info.path_info_count);

    for (NvU32 i = 0; i < displayconfig_path_info.path_info_count; i++) {
        printfln_err("Num total displays in adapter: %ld", displayconfig_path_info.path_info[i].targetInfoCount);

        for (NvU32 j = 0; j < displayconfig_path_info.path_info[i].targetInfoCount; j++) {
            target_info = &displayconfig_path_info.path_info[i].targetInfo[j];
            source_mode_info = &displayconfig_path_info.path_info[i].sourceModeInfo[j];

            if (display_id != 0 && target_info->displayId != display_id) {
                continue;
            }

            printfln_out("--------------------------------");
            printfln_out("Adapter source ID %ld", displayconfig_path_info.path_info[i].sourceId);
            printfln_out("Display ID: %lX", target_info->displayId);
            printfln_out("Resolution width: %ld", source_mode_info->resolution.width);
            printfln_out("Resolution height: %ld", source_mode_info->resolution.height);
            printfln_out("Color depth: %d", source_mode_info->colorFormat);
            printfln_out("Position x: %d", source_mode_info->position.x);
            printfln_out("Position y: %d", source_mode_info->position.y);
            printfln_out("Spanning orientation: %d", source_mode_info->spanningOrientation);
            printfln_out("GDI primary: %d", source_mode_info->bGDIPrimary);
            printfln_out("SLI focus: %d", source_mode_info->bSLIFocus);

            if (target_info->details) {
                switch (target_info->details->rotation) {
                    case NV_ROTATE_0:
                        printfln_out("Rotation: 0 degrees");
                        break;
                    case NV_ROTATE_90:
                        printfln_out("Rotation: 90 degrees");
                        break;
                    case NV_ROTATE_180:
                        printfln_out("Rotation: 180 degrees");
                        break;
                    case NV_ROTATE_270:
                        printfln_out("Rotation: 270 degrees");
                        break;
                    case NV_ROTATE_IGNORED:
                        printfln_out("Rotation: ignored");
                        break;
                    default:
                        printfln_out("Rotation: unknown (%d)", target_info->details->rotation);
                        break;
                }

                switch (target_info->details->scaling) {
                    case NV_SCALING_DEFAULT:
                        printfln_out("Scaling: default");
                        break;
                    case NV_SCALING_GPU_SCALING_TO_CLOSEST:
                        printfln_out("Scaling: GPU scaling to closest");
                        break;
                    case NV_SCALING_GPU_SCALING_TO_NATIVE:
                        printfln_out("Scaling: GPU scaling to native");
                        break;
                    case NV_SCALING_GPU_SCANOUT_TO_NATIVE:
                        printfln_out("Scaling: GPU scanout to native");
                        break;
                    case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_NATIVE:
                        printfln_out("Scaling: GPU scaling to aspect scanout to native");
                        break;
                    case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_CLOSEST:
                        printfln_out("Scaling: GPU scaling to aspect scanout to closest");
                        break;
                    case NV_SCALING_GPU_SCANOUT_TO_CLOSEST:
                        printfln_out("Scaling: GPU scanout to closest");
                        break;
                    case NV_SCALING_GPU_INTEGER_ASPECT_SCALING:
                        printfln_out("Scaling: GPU integer aspect scaling");
                        break;
                    default:
                        printfln_out("Scaling: unknown (%d)", target_info->details->scaling);
                        break;
                }

                printfln_out("Refresh rate (non interlaced): %ld", target_info->details->refreshRate1K * 1000);
                printfln_out("Interlaced: %d", target_info->details->interlaced);
                printfln_out("Primary: %d", target_info->details->primary);
                printfln_out("Disable virtual mode support: %d", target_info->details->disableVirtualModeSupport);
                printfln_out("Is preferred unscaled target: %d", target_info->details->isPreferredUnscaledTarget);

                switch (target_info->details->connector) {
                    case NVAPI_GPU_CONNECTOR_VGA_15_PIN:
                        printfln_out("Connector: VGA 15 Pin");
                        break;
                    case NVAPI_GPU_CONNECTOR_TV_COMPOSITE:
                        printfln_out("Connector: TV Composite");
                        break;
                    case NVAPI_GPU_CONNECTOR_TV_SVIDEO:
                        printfln_out("Connector: TV S-Video");
                        break;
                    case NVAPI_GPU_CONNECTOR_TV_HDTV_COMPONENT:
                        printfln_out("Connector: TV HDTV Component");
                        break;
                    case NVAPI_GPU_CONNECTOR_TV_SCART:
                        printfln_out("Connector: TV SCART");
                        break;
                    case NVAPI_GPU_CONNECTOR_TV_COMPOSITE_SCART_ON_EIAJ4120:
                        printfln_out("Connector: TV Composite SCART on EIAJ4120");
                        break;
                    case NVAPI_GPU_CONNECTOR_TV_HDTV_EIAJ4120:
                        printfln_out("Connector: TV HDTV EIAJ4120");
                        break;
                    case NVAPI_GPU_CONNECTOR_PC_POD_HDTV_YPRPB:
                        printfln_out("Connector: PC POD HDTV YPRPB");
                        break;
                    case NVAPI_GPU_CONNECTOR_PC_POD_SVIDEO:
                        printfln_out("Connector: PC POD S-Video");
                        break;
                    case NVAPI_GPU_CONNECTOR_PC_POD_COMPOSITE:
                        printfln_out("Connector: PC POD Composite");
                        break;
                    case NVAPI_GPU_CONNECTOR_DVI_I_TV_SVIDEO:
                        printfln_out("Connector: DVI-I TV S-Video");
                        break;
                    case NVAPI_GPU_CONNECTOR_DVI_I_TV_COMPOSITE:
                        printfln_out("Connector: DVI-I TV Composite");
                        break;
                    case NVAPI_GPU_CONNECTOR_DVI_I:
                        printfln_out("Connector: DVI-I");
                        break;
                    case NVAPI_GPU_CONNECTOR_DVI_D:
                        printfln_out("Connector: DVI-D");
                        break;
                    case NVAPI_GPU_CONNECTOR_ADC:
                        printfln_out("Connector: ADC");
                        break;
                    case NVAPI_GPU_CONNECTOR_LFH_DVI_I_1:
                        printfln_out("Connector: LFH DVI-I 1");
                        break;
                    case NVAPI_GPU_CONNECTOR_LFH_DVI_I_2:
                        printfln_out("Connector: LFH DVI-I 2");
                        break;
                    case NVAPI_GPU_CONNECTOR_SPWG:
                        printfln_out("Connector: SPWG");
                        break;
                    case NVAPI_GPU_CONNECTOR_OEM:
                        printfln_out("Connector: OEM");
                        break;
                    case NVAPI_GPU_CONNECTOR_DISPLAYPORT_EXTERNAL:
                        printfln_out("Connector: DisplayPort External");
                        break;
                    case NVAPI_GPU_CONNECTOR_DISPLAYPORT_INTERNAL:
                        printfln_out("Connector: DisplayPort Internal");
                        break;
                    case NVAPI_GPU_CONNECTOR_DISPLAYPORT_MINI_EXT:
                        printfln_out("Connector: DisplayPort Mini External");
                        break;
                    case NVAPI_GPU_CONNECTOR_HDMI_A:
                        printfln_out("Connector: HDMI A");
                        break;
                    case NVAPI_GPU_CONNECTOR_HDMI_C_MINI:
                        printfln_out("Connector: HDMI C Mini");
                        break;
                    case NVAPI_GPU_CONNECTOR_LFH_DISPLAYPORT_1:
                        printfln_out("Connector: LFH DisplayPort 1");
                        break;
                    case NVAPI_GPU_CONNECTOR_LFH_DISPLAYPORT_2:
                        printfln_out("Connector: LFH DisplayPort 2");
                        break;
                    case NVAPI_GPU_CONNECTOR_VIRTUAL_WFD:
                        printfln_out("Connector: Virtual WFD");
                        break;
                    case NVAPI_GPU_CONNECTOR_USB_C:
                        printfln_out("Connector: USB C");
                        break;
                    default:
                        printfln_out("Connector: unknown (%d)", target_info->details->connector);
                        break;
                }

                switch (target_info->details->tvFormat) {
                    case NV_DISPLAY_TV_FORMAT_NONE:
                        printfln_out("TV format: none");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_NTSCM:
                        printfln_out("TV format: SD NTSC-M");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_NTSCJ:
                        printfln_out("TV format: SD NTSC-J");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_PALM:
                        printfln_out("TV format: SD PAL-M");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_PALBDGH:
                        printfln_out("TV format: SD PAL-BDGH");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_PALN:
                        printfln_out("TV format: SD PAL-N");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_PALNC:
                        printfln_out("TV format: SD PAL-NC");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_576i:
                        printfln_out("TV format: SD 576i");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_480i:
                        printfln_out("TV format: SD 480i");
                        break;
                    case NV_DISPLAY_TV_FORMAT_ED_480p:
                        printfln_out("TV format: ED 480p");
                        break;
                    case NV_DISPLAY_TV_FORMAT_ED_576p:
                        printfln_out("TV format: ED 576p");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_720p:
                        printfln_out("TV format: HD 720p");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_1080i:
                        printfln_out("TV format: HD 1080i");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_1080p:
                        printfln_out("TV format: HD 1080p");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_720p50:
                        printfln_out("TV format: HD 720p50");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_1080p24:
                        printfln_out("TV format: HD 1080p24");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_1080i50:
                        printfln_out("TV format: HD 1080i50");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_1080p50:
                        printfln_out("TV format: HD 1080p50");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp30_3840:
                        printfln_out("TV format: UHD 4Kp30 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp25_3840:
                        printfln_out("TV format: UHD 4Kp25 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp24_3840:
                        printfln_out("TV format: UHD 4Kp24 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp24_SMPTE:
                        printfln_out("TV format: UHD 4Kp24 SMPTE");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp50_3840:
                        printfln_out("TV format: UHD 4Kp50 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp60_3840:
                        printfln_out("TV format: UHD 4Kp60 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp30_4096:
                        printfln_out("TV format: UHD 4Kp30 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp25_4096:
                        printfln_out("TV format: UHD 4Kp25 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp24_4096:
                        printfln_out("TV format: UHD 4Kp24 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp50_4096:
                        printfln_out("TV format: UHD 4Kp50 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp60_4096:
                        printfln_out("TV format: UHD 4Kp60 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp24_7680:
                        printfln_out("TV format: UHD 8Kp24 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp25_7680:
                        printfln_out("TV format: UHD 8Kp25 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp30_7680:
                        printfln_out("TV format: UHD 8Kp30 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp48_7680:
                        printfln_out("TV format: UHD 8Kp48 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp50_7680:
                        printfln_out("TV format: UHD 8Kp50 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp60_7680:
                        printfln_out("TV format: UHD 8Kp60 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp100_7680:
                        printfln_out("TV format: UHD 8Kp100 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_8Kp120_7680:
                        printfln_out("TV format: UHD 8Kp120 7680");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp48_3840:
                        printfln_out("TV format: UHD 4Kp48 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp48_4096:
                        printfln_out("TV format: UHD 4Kp48 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp100_3840:
                        printfln_out("TV format: UHD 4Kp100 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp100_4096:
                        printfln_out("TV format: UHD 4Kp100 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp120_4096:
                        printfln_out("TV format: UHD 4Kp120 4096");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp120_3840:
                        printfln_out("TV format: UHD 4Kp120 3840");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp100_5120:
                        printfln_out("TV format: UHD 4Kp100 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp120_5120:
                        printfln_out("TV format: UHD 4Kp120 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp24_5120:
                        printfln_out("TV format: UHD 4Kp24 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp25_5120:
                        printfln_out("TV format: UHD 4Kp25 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp30_5120:
                        printfln_out("TV format: UHD 4Kp30 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp48_5120:
                        printfln_out("TV format: UHD 4Kp48 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp50_5120:
                        printfln_out("TV format: UHD 4Kp50 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_4Kp60_5120:
                        printfln_out("TV format: UHD 4Kp60 5120");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp24_10240:
                        printfln_out("TV format: UHD 10Kp24 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp25_10240:
                        printfln_out("TV format: UHD 10Kp25 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp30_10240:
                        printfln_out("TV format: UHD 10Kp30 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp48_10240:
                        printfln_out("TV format: UHD 10Kp48 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp50_10240:
                        printfln_out("TV format: UHD 10Kp50 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp60_10240:
                        printfln_out("TV format: UHD 10Kp60 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp100_10240:
                        printfln_out("TV format: UHD 10Kp100 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_UHD_10Kp120_10240:
                        printfln_out("TV format: UHD 10Kp120 10240");
                        break;
                    case NV_DISPLAY_TV_FORMAT_SD_OTHER:
                        printfln_out("TV format: SD Other");
                        break;
                    case NV_DISPLAY_TV_FORMAT_ED_OTHER:
                        printfln_out("TV format: ED Other");
                        break;
                    case NV_DISPLAY_TV_FORMAT_HD_OTHER:
                        printfln_out("TV format: HD Other");
                        break;
                    case NV_DISPLAY_TV_FORMAT_ANY:
                        printfln_out("TV format: Any");
                        break;
                    default:
                        printfln_out("TV format: unknown (%d)", target_info->details->tvFormat);
                        break;
                }

                switch (target_info->details->timingOverride) {
                    case NV_TIMING_OVERRIDE_CURRENT:
                        printfln_out("Timing override: current");
                        break;
                    case NV_TIMING_OVERRIDE_AUTO:
                        printfln_out("Timing override: auto");
                        break;
                    case NV_TIMING_OVERRIDE_EDID:
                        printfln_out("Timing override: EDID");
                        break;
                    case NV_TIMING_OVERRIDE_DMT:
                        printfln_out("Timing override: VESA DMT");
                        break;
                    case NV_TIMING_OVERRIDE_DMT_RB:
                        printfln_out("Timing override: VESA DMT RB");
                        break;
                    case NV_TIMING_OVERRIDE_CVT:
                        printfln_out("Timing override: VESA CVT");
                        break;
                    case NV_TIMING_OVERRIDE_CVT_RB:
                        printfln_out("Timing override: VESA CVT RB");
                        break;
                    case NV_TIMING_OVERRIDE_GTF:
                        printfln_out("Timing override: VESA GTF");
                        break;
                    case NV_TIMING_OVERRIDE_EIA861:
                        printfln_out("Timing override: EIA 861x pre-defined timing");
                        break;
                    case NV_TIMING_OVERRIDE_ANALOG_TV:
                        printfln_out("Timing override: analog SD/HDTV timing");
                        break;
                    case NV_TIMING_OVERRIDE_CUST:
                        printfln_out("Timing override: NV custom timings");
                        break;
                    case NV_TIMING_OVERRIDE_NV_PREDEFINED:
                        printfln_out("Timing override: NV pre-defined timing (basically the PsF timings)");
                        break;
                    case NV_TIMING_OVERRIDE_NV_ASPR:
                        printfln_out("Timing override: NV ASPR");
                        break;
                    case NV_TIMING_OVERRIDE_SDI:
                        printfln_out("Timing override: SDI");
                        break;
                    case NV_TIMING_OVRRIDE_MAX:
                        printfln_out("Timing override: max");
                        break;
                    default:
                        printfln_out("Timing override: unknown (%d)", target_info->details->timingOverride);
                        break;
                }

                printfln_out("Override custom (backend raster) timing");

                printfln_out("Horizontal visible: %d", target_info->details->timing.HVisible);
                printfln_out("Horizontal border: %d", target_info->details->timing.HBorder);
                printfln_out("Horizontal front porch: %d", target_info->details->timing.HFrontPorch);
                printfln_out("Horizontal sync width: %d", target_info->details->timing.HSyncWidth);
                printfln_out("Horizontal total: %d", target_info->details->timing.HTotal);
                printfln_out("Horizontal sync polarity: %s", target_info->details->timing.HSyncPol == 1 ? "negative" : "positive");

                printfln_out("Vertical visible: %d", target_info->details->timing.VVisible);
                printfln_out("Vertical border: %d", target_info->details->timing.VBorder);
                printfln_out("Vertical front porch: %d", target_info->details->timing.VFrontPorch);
                printfln_out("Vertical sync width: %d", target_info->details->timing.VSyncWidth);
                printfln_out("Vertical total: %d", target_info->details->timing.VTotal);
                printfln_out("Vertical sync polarity: %s", target_info->details->timing.VSyncPol == 1 ? "negative" : "positive");

                printfln_out("Interlaced: %s", target_info->details->timing.interlaced == 1 ? "interlaced" : "progressive");
                printfln_out("Pixel clock: %lu kHz", target_info->details->timing.pclk * 10);

                printfln_out("Flag: 0x%lX", target_info->details->timing.etc.flag);
                printfln_out("Logical refresh rate to present: %d", target_info->details->timing.etc.rr);
                printfln_out("Physical vertical refresh rate: %f Hz", target_info->details->timing.etc.rrx1k * 0.001);
                printfln_out("Display aspect ratio: %lu:%lu", target_info->details->timing.etc.aspect >> 16, target_info->details->timing.etc.aspect & 0xFFFF);
                printfln_out("Bit-wise pixel repetition factor (1 = no pixel repetition, 2 = each pixel repeats twice horizontally, etc.): %d", target_info->details->timing.etc.rep);
                printfln_out("Timing status: 0x%lX", target_info->details->timing.etc.status);
                printfln_out("Timing name: %s", target_info->details->timing.etc.name);
            }

            printfln_out("--------------------------------");
        }
    }

    _display_config_free(&displayconfig_path_info);

    return true;
}

static bool _custom_resolution_set(
        const nv_api_t *nv_api,
        NvU32 display_id,
        uint16_t screen_width,
        uint16_t screen_height,
        float screen_refresh_rate,
        uint8_t test_only_timeout_sec)
{
    NV_CUSTOM_DISPLAY custom_display;
    NV_TIMING_FLAG flag;
    NV_TIMING_INPUT timing;
    NvAPI_Status status;
    
    assert(nv_api);

    memset(&custom_display, 0, sizeof(NV_CUSTOM_DISPLAY));
    memset(&flag, 0, sizeof(NV_TIMING_FLAG));
    memset(&timing, 0, sizeof(NV_TIMING_INPUT));

    custom_display.version = NV_CUSTOM_DISPLAY_VER;
    custom_display.width = screen_width;
    custom_display.height = screen_height;
    custom_display.depth = 32;
    custom_display.colorFormat = NV_FORMAT_A8R8G8B8;
    custom_display.srcPartition.x = 0;
    custom_display.srcPartition.y = 0;
    custom_display.srcPartition.w = 1;
    custom_display.srcPartition.h = 1;
    custom_display.xRatio = 1;
    custom_display.yRatio = 1;

    timing.version = NV_TIMING_INPUT_VER;
    timing.height = screen_height;
    timing.width = screen_width;
    timing.rr = screen_refresh_rate;
    timing.flag = flag;
    timing.type = NV_TIMING_OVERRIDE_CVT_RB;

    printfln_err("Calculating custom display timing for display ID 0x%lX", display_id);
    
    status = nv_api->NvAPI_DISP_GetTiming(display_id, &timing, &custom_display.timing);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Calculating custom display timing");
        return false;
    }

    status = nv_api->NvAPI_DISP_TryCustomDisplay(&display_id, 1, &custom_display);

    if (status != NVAPI_OK) {
        PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Trying custom display configuration");
        return false;
    }

    if (test_only_timeout_sec == 0) {
        printfln_err("Persisting custom display configuration");

        status = nv_api->NvAPI_DISP_SaveCustomDisplay(&display_id, 1, true, true);

        if (status != NVAPI_OK) {
            PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Saving custom display configuration");
            return false;
        }

        printfln_err("Custom display configuration persisted");
    } else {
        printfln_err("Trying custom display configuration for %d seconds...", test_only_timeout_sec);

        Sleep(test_only_timeout_sec * 1000);

        printfln_err("Reverting custom display configuration, not persisting");

        status = nv_api->NvAPI_DISP_RevertCustomDisplayTrial(&display_id, 1);

        if (status != NVAPI_OK) {
            PRINT_ERR_WITH_NVAPI_MESSAGE(status, "ERROR: Reverting custom display configuration");
            return false;
        }
    }

    return true;
}

static bool _display_config_gpu_scaling_enable_filter(NV_DISPLAYCONFIG_PATH_TARGET_INFO *target_info)
{
    if (target_info->details == NULL) {
        printfln_err("ERROR: No details found for target info");
        return false;
    }

    if (target_info->details->scaling == NV_SCALING_GPU_SCALING_TO_NATIVE) {
        printfln_err("GPU scaling to native resolution is already enabled");
        return false;
    } else {
        target_info->details->scaling = NV_SCALING_GPU_SCALING_TO_NATIVE;
        printfln_err("Enabling GPU scaling to native resolution");
        return true;
    }
}

static bool _display_config_gpu_scaling_to_native_resolution_enable(const nv_api_t *nv_api, NvU32 display_id)
{
    displayconfig_path_info_t displayconfig_path_info;
    bool changes_to_save;

    assert(nv_api);

    if (!_display_config_get(nv_api, &displayconfig_path_info)) {
        return false;
    }

    changes_to_save = _display_config_visit(&displayconfig_path_info, display_id, _display_config_gpu_scaling_enable_filter);

    if (changes_to_save) {
        if (!_display_config_set(nv_api, &displayconfig_path_info)) {
            _display_config_free(&displayconfig_path_info);
            return false;
        }
    }

    _display_config_free(&displayconfig_path_info);

    return true;
}

// -------------------------------------------------------------------------------------------------

static void _print_synopsis()
{
    printfln_err("Usage: nvgpu <command> <subcommand>");
    printfln_err("Commands:");
    printfln_err("  nv");
    printfln_err("    info                            Print information about the NVAPI module and driver");
    printfln_err("");
    printfln_err("  profile");
    printfln_err("    create <profile_name>         Create a new driver profile with the given name");
    printfln_err("    delete <profile_name>         Delete an existing driver profile");
    printfln_err("    application-add <profile_name> <application_name>");
    printfln_err("                                  Add an application to the driver profile. This will apply the profile to the application when the driver detects a process being launched, e.g. MyApplication.exe");
    printfln_err("    gsync-disable <profile_name>  Disable G-SYNC for the driver profile");
    printfln_err("    gpu-power-state-max <profile_name>  Set GPU power state to maximum for the driver profile");
    printfln_err("");
    printfln_err("  display");
    printfln_err("    primary-display-id            Print the ID of the primary display");
    printfln_err("    list                          List all connected displays and their display IDs");
    printfln_err("    config [display_id]           Print the current display configurations. Optionally, specify a display ID to get the configuration of that display only");
    printfln_err("    custom-resolution-set <display_id> <screen_width> <screen_height> <screen_refresh_rate>");
    printfln_err("                                  Set a custom display mode with the given parameters for the given display ID. The settings are persisted immediately. Ensure you tested these before with the custom-display-test command.");
    printfln_err("    custom-resolution-test <display_id> <screen_width> <screen_height> <screen_refresh_rate> [--test-timeout-secs n]");
    printfln_err("                                  Test a custom display mode for a limited amount of time. This will revert the display mode after the given amount of seconds and not persist the changes.");
    printfln_err("                                  test-timeout-secs: Optional. Number of seconds to test the custom display mode for. Default is 10 seconds.");
    printfln_err("    gpu-scaling-to-native-resolution-enable <display_id> Enable GPU scaling to native resolution for the given display ID");
}

static bool _cmd_nv_info(const nv_api_t *nv_api)
{
    return _nv_info(nv_api);
}

static bool _cmd_profile_create(const nv_api_t *nv_api, int argc, char **argv)
{
    const char *profile_name;

    if (argc < 1) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    profile_name = argv[0];

    return _profile_create(nv_api, profile_name);
}

static bool _cmd_profile_delete(const nv_api_t *nv_api, int argc, char **argv)
{
    const char *profile_name;

    if (argc < 1) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    profile_name = argv[0];

    return _profile_delete(nv_api, profile_name);
}

static bool _cmd_profile_application_add(const nv_api_t *nv_api, int argc, char **argv)
{
    const char *profile_name;
    const char *application_name;

    if (argc < 2) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    profile_name = argv[0];
    application_name = argv[1];

    return _profile_application_add(nv_api, profile_name, application_name);
}

static bool _cmd_profile_gsync_disable(const nv_api_t *nv_api, int argc, char **argv)
{
    const char *profile_name;

    if (argc < 1) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    profile_name = argv[0];

    return _profile_gsync_disable(nv_api, profile_name);
}

static bool _cmd_profile_gpu_power_state_max(const nv_api_t *nv_api, int argc, char **argv)
{
    const char *profile_name;

    if (argc < 1) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    profile_name = argv[0];

    return _profile_gpu_power_state_max(nv_api, profile_name);
}

static bool _cmd_display_primary_display_id(const nv_api_t *nv_api)
{
    return _display_primary_display_id(nv_api);
}

static bool _cmd_display_list(const nv_api_t *nv_api)
{
    return _displays_list(nv_api);
}

static bool _cmd_display_config(const nv_api_t *nv_api, int argc, char **argv)
{
    uint32_t display_id;

    if (argc > 0) {
        if (argv[0][0] == '0' && argv[0][1] == 'x') {
            display_id = strtoul(argv[0] + 2, NULL, 16);
        } else {
            display_id = strtoul(argv[0], NULL, 10);
        }
    } else {
        display_id = 0;
    }

    return _display_config_print(nv_api, display_id);
}

static bool _cmd_custom_resolution_set(const nv_api_t *nv_api, int argc, char **argv)
{
    uint32_t display_id;
    uint16_t screen_width;
    uint16_t screen_height;
    float screen_refresh_rate;

    if (argc < 4) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    if (argv[0][0] == '0' && argv[0][1] == 'x') {
        display_id = strtoul(argv[0] + 2, NULL, 16);
    } else {
        display_id = strtoul(argv[0], NULL, 10);
    }
    
    screen_width = atoi(argv[1]);
    screen_height = atoi(argv[2]);
    screen_refresh_rate = atof(argv[3]);

    printf_err("Setting custom resolution for display ID 0x%X: %dx%d@%f\n", display_id, screen_width, screen_height, screen_refresh_rate);

    return _custom_resolution_set(
        nv_api,
        display_id,
        screen_width,
        screen_height,
        screen_refresh_rate,
        0);
}

static bool _cmd_custom_resolution_test(const nv_api_t *nv_api, int argc, char **argv)
{
    uint32_t display_id;
    uint16_t screen_width;
    uint16_t screen_height;
    float screen_refresh_rate;
    uint8_t test_only_timeout_sec;

    if (argc < 5) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }

    if (argv[0][0] == '0' && argv[0][1] == 'x') {
        display_id = strtoul(argv[0] + 2, NULL, 16);
    } else {
        display_id = strtoul(argv[0], NULL, 10);
    }

    screen_width = atoi(argv[1]);
    screen_height = atoi(argv[2]);
    screen_refresh_rate = atof(argv[3]);

    // Sane defaults for optional parameters
    test_only_timeout_sec = 10;

    for (int i = 4; i < argc; i++) {
        if (!strcmp(argv[i], "--test-timeout-secs")) {
            if (i + 1 < argc) {
                test_only_timeout_sec = atoi(argv[++i]);

                if (test_only_timeout_sec == 0) {
                    printfln_err("ERROR: Time out parameter must be greater than 0");
                    return false;
                }
            } else {
                printfln_err("ERROR: Missing argument for --test-timeout-secs");
                return false;
            }
        }
    }

    printf_err("Testing custom resolution for display ID %u: %dx%d@%f, timeout %d seconds\n", display_id, screen_width, screen_height, screen_refresh_rate, test_only_timeout_sec);

    return _custom_resolution_set(
        nv_api,
        display_id,
        screen_width,
        screen_height,
        screen_refresh_rate,
        test_only_timeout_sec);
}

static bool _cmd_display_gpu_scaling_to_native_resolution_enable(const nv_api_t *nv_api, int argc, char **argv)
{
    uint32_t display_id;

    if (argc < 1) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        return false;
    }
    
    if (argv[0][0] == '0' && argv[0][1] == 'x') {
        display_id = strtoul(argv[0] + 2, NULL, 16);
    } else {
        display_id = strtoul(argv[0], NULL, 10);
    }

    return _display_config_gpu_scaling_to_native_resolution_enable(nv_api, display_id); 
}

// -------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    const char *command;
    const char *sub_command;
    nv_module_t *nv_module;
    nv_api_t nv_api;
    NvAPI_Status status;
    bool result;

    if (argc < 3) {
        _print_synopsis();
        printfln_err("ERROR: Insufficient arguments");
        exit(1);
    }

    command = argv[1];
    sub_command = argv[2];

    printfln_err("Loading NVAPI module...");

    nv_module_load(&nv_module);
    nv_module_api_get(nv_module, &nv_api);

    printfln_err("Initializing NVAPI...");

    status = nv_api.NvAPI_Initialize();

    if (status != NVAPI_OK) {
        NvAPI_ShortString error_str;
        nv_api.NvAPI_GetErrorMessage(status, error_str);
        fprintf(stderr, "ERROR: Initializing NVAPI, reason: %s\n", error_str);

        nv_module_free(&nv_module);
        exit(1);
    }

    printfln_err("Running command: %s %s", command, sub_command);

    if (!strcmp(command, "nv")) {
        if (!strcmp(sub_command, "info")) {
            result = _cmd_nv_info(&nv_api);
        } else {
            printfln_err("ERROR: Unknown sub-command: %s", sub_command);
            _print_synopsis();
            result = false;
        }
    } else if (!strcmp(command, "profile")) {
        _ensure_drs_settings_folder_exists();
        
        if (!strcmp(sub_command, "create")) {
            result = _cmd_profile_create(&nv_api, argc - 3, argv + 3);
        } else if (!strcmp(sub_command, "delete")) {
            result = _cmd_profile_delete(&nv_api, argc - 3, argv + 3);
        } else if (!strcmp(sub_command, "application-add")) {
            result = _cmd_profile_application_add(&nv_api, argc - 3, argv + 3);
        } else if (!strcmp(sub_command, "gsync-disable")) {
            result = _cmd_profile_gsync_disable(&nv_api, argc - 3, argv + 3);
        } else if (!strcmp(sub_command, "gpu-power-state-max")) {
            result = _cmd_profile_gpu_power_state_max(&nv_api, argc - 3, argv + 3);
        } else {
            printfln_err("ERROR: Unknown sub-command: %s", sub_command);
            _print_synopsis();
            result = false;
        }
    } else if (!strcmp(command, "display")) {
        if (!strcmp(sub_command, "primary-display-id")) {
            result = _cmd_display_primary_display_id(&nv_api);
        } else if (!strcmp(sub_command, "list")) {
            result = _cmd_display_list(&nv_api);
        } else if (!strcmp(sub_command, "config")) {
            result = _cmd_display_config(&nv_api, argc - 3, argv + 3);
        } else if (!strcmp(sub_command, "custom-resolution-set")) {
            result = _cmd_custom_resolution_set(&nv_api, argc - 3, argv + 3);
        } else if (!strcmp(sub_command, "custom-resolution-test")) {
            result = _cmd_custom_resolution_test(&nv_api, argc - 3, argv + 3);
        } else if (!strcmp(sub_command, "gpu-scaling-to-native-resolution-enable")) {
            result = _cmd_display_gpu_scaling_to_native_resolution_enable(&nv_api, argc - 3, argv + 3);
        } else {
            printfln_err("ERROR: Unknown sub-command: %s", sub_command);
            _print_synopsis();
            result = false;
        }
    } else {
        printfln_err("ERROR: Unknown command: %s", command);
        _print_synopsis();
        result = false;
    }

    status = nv_api.NvAPI_Unload();

    if (status != NVAPI_OK) {
        NvAPI_ShortString error_str;
        nv_api.NvAPI_GetErrorMessage(status, error_str);
        printfln_err("ERROR: Unloading NVAPI, reason: %s", error_str);
    }

    nv_module_free(&nv_module);
    
    if (result) {
        printfln_err("Command completed successfully");
        exit(0);
    } else {
        printfln_err("Command failed");
        exit(1);
    }
}
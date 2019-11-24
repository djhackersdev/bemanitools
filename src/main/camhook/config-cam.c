#include "cconfig/cconfig-util.h"

#include "camhook/config-cam.h"

#include "util/log.h"

#define CAMHOOK_CONFIG_CAM_DISABLE_EMU_KEY "cam.disable_emu"
#define CAMHOOK_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE false

const char *camhook_config_device_id_keys[CAMHOOK_CONFIG_CAM_MAX] = {
    "cam.device_id1",
    "cam.device_id2",
};

const char *camhook_config_device_default_values[CAMHOOK_CONFIG_CAM_MAX] = {
    "",
    "",
};

void camhook_config_cam_init(struct cconfig *config, size_t num_cams)
{
    cconfig_util_set_bool(
        config,
        CAMHOOK_CONFIG_CAM_DISABLE_EMU_KEY,
        CAMHOOK_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE,
        "Disables the camera emulation");

    for (size_t i = 0; i < num_cams; ++i) {
        cconfig_util_set_str(
            config,
            camhook_config_device_id_keys[i],
            camhook_config_device_default_values[i],
            "Override camera device ID detection (copy from device manager, do "
            "not escape)");
    }
}

void camhook_config_cam_get(
    struct camhook_config_cam *config_cam,
    struct cconfig *config,
    size_t num_cams)
{
    config_cam->num_devices = num_cams;

    if (!cconfig_util_get_bool(
            config,
            CAMHOOK_CONFIG_CAM_DISABLE_EMU_KEY,
            &config_cam->disable_emu,
            CAMHOOK_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            CAMHOOK_CONFIG_CAM_DISABLE_EMU_KEY,
            CAMHOOK_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE);
    }
    for (size_t i = 0; i < num_cams; ++i) {
        if (!cconfig_util_get_str(
                config,
                camhook_config_device_id_keys[i],
                config_cam->device_id[i],
                sizeof(config_cam->device_id[i]) - 1,
                camhook_config_device_default_values[i])) {
            log_warning(
                "Invalid value for key '%s' specified, fallback "
                "to default '%s'",
                camhook_config_device_id_keys[i],
                camhook_config_device_default_values[i]);
        }
    }
}

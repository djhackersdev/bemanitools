#include "cconfig/cconfig-util.h"

#include "core/config-ext.h"

#include "camhook/config-cam.h"

#include "iface-core/log.h"

#include "util/str.h"

#define CAMHOOK_CONFIG_CAM_DISABLE_EMU_KEY "cam.disable_emu"
#define CAMHOOK_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE false

#define CAMHOOK_CONFIG_CAM_PORT_LAYOUT_KEY "cam.port_layout"
#define CAMHOOK_CONFIG_CAM_DEFAULT_PORT_LAYOUT_VALUE 0

// the following four arrays are based on CAMHOOK_CONFIG_CAM_MAX
// please insert more elements if more cams are added
const char *camhook_config_disable_camera[CAMHOOK_CONFIG_CAM_MAX] = {
    "cam.disable_camera1",
    "cam.disable_camera2",
};

const int camhook_config_disable_camera_default_values[CAMHOOK_CONFIG_CAM_MAX] =
    {
        false,
        false,
};

const char *camhook_config_device_id_keys[CAMHOOK_CONFIG_CAM_MAX] = {
    "cam.device_id1",
    "cam.device_id2",
};

const char *camhook_config_device_default_values[CAMHOOK_CONFIG_CAM_MAX] = {
    "",
    "",
};

void camhook_config_cam_init(struct cconfig *config, size_t num_cams, bool use_port_layout)
{
    cconfig_util_set_bool(
        config,
        CAMHOOK_CONFIG_CAM_DISABLE_EMU_KEY,
        CAMHOOK_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE,
        "Disables the camera emulation");

    if (use_port_layout) {
        cconfig_util_set_int(
            config,
            CAMHOOK_CONFIG_CAM_PORT_LAYOUT_KEY,
            CAMHOOK_CONFIG_CAM_DEFAULT_PORT_LAYOUT_VALUE,
            "Camera port layout (0 = LDJ, 1 = CLDJ/TDJ-JA, 2 = TDJ-JB)");
    }

    for (size_t i = 0; i < num_cams; ++i) {
        cconfig_util_set_bool(
            config,
            camhook_config_disable_camera[i],
            camhook_config_disable_camera_default_values[i],
            "Disable camera");
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
    size_t num_cams,
    bool use_port_layout)
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

    if (use_port_layout) {
        if (!cconfig_util_get_int(
                config,
                CAMHOOK_CONFIG_CAM_PORT_LAYOUT_KEY,
                &config_cam->port_layout,
                CAMHOOK_CONFIG_CAM_DEFAULT_PORT_LAYOUT_VALUE)) {
            log_warning(
                "Invalid value for key '%s' specified, fallback "
                "to default '%d'",
                CAMHOOK_CONFIG_CAM_PORT_LAYOUT_KEY,
                CAMHOOK_CONFIG_CAM_DEFAULT_PORT_LAYOUT_VALUE);
        }
    }

    for (size_t i = 0; i < num_cams; ++i) {
        if (!cconfig_util_get_bool(
                config,
                camhook_config_disable_camera[i],
                &config_cam->disable_camera[i],
                camhook_config_disable_camera_default_values[i])) {
            log_warning(
                "Invalid value for key '%s' specified, fallback "
                "to default '%d'",
                camhook_config_disable_camera[i],
                camhook_config_disable_camera_default_values[i]);
        }
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

void camhook_config_cam_get2(
    const bt_core_config_t *config,
    struct camhook_config_cam *config_cam,
    size_t num_cams,
    bool use_port_layout)
{
    char key[32];

    bt_core_config_ext_bool_get(config, "camera/disable_emu", &config_cam->disable_emu);

    if (use_port_layout) {
        bt_core_config_ext_s32_get(config, "camera/port_layout", &config_cam->port_layout);
    }

    for (size_t i = 0; i < num_cams; ++i) {
        str_format(key, sizeof(key), "camera/device_%d/disable", i);
        bt_core_config_ext_bool_get(config, key, &config_cam->disable_camera[i]);

        str_format(key, sizeof(key), "camera/device_%d/id", i);
        bt_core_config_ext_str_get(config, key, config_cam->device_id[i], sizeof(config_cam->device_id[i]) - 1);
    }   
}
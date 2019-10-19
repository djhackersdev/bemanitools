#include "cconfig/cconfig-util.h"

#include "iidxhook8/config-cam.h"

#include "util/log.h"

#define IIDXHOOK8_CONFIG_CAM_DISABLE_EMU_KEY "cam.disable_emu"
#define IIDXHOOK8_CONFIG_CAM_DEVICE_ID1_KEY "cam.device_id1"
#define IIDXHOOK8_CONFIG_CAM_DEVICE_ID2_KEY "cam.device_id2"

#define IIDXHOOK8_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE false
#define IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID1_VALUE ""
#define IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID2_VALUE ""

void iidxhook8_config_cam_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK8_CONFIG_CAM_DISABLE_EMU_KEY,
        IIDXHOOK8_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE,
        "Disables the camera emulation");

    cconfig_util_set_str(
        config,
        IIDXHOOK8_CONFIG_CAM_DEVICE_ID1_KEY,
        IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID1_VALUE,
        "Override camera device ID 1 detection (copy from device manager, "
        "do not escape)");

    cconfig_util_set_str(
        config,
        IIDXHOOK8_CONFIG_CAM_DEVICE_ID2_KEY,
        IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID2_VALUE,
        "Override camera device ID 2 detection (copy from device manager, "
        "do not escape)");
}

void iidxhook8_config_cam_get(
    struct iidxhook8_config_cam *config_cam, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK8_CONFIG_CAM_DISABLE_EMU_KEY,
            &config_cam->disable_emu,
            IIDXHOOK8_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK8_CONFIG_CAM_DISABLE_EMU_KEY,
            IIDXHOOK8_CONFIG_CAM_DEFAULT_DISABLE_EMU_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            IIDXHOOK8_CONFIG_CAM_DEVICE_ID1_KEY,
            config_cam->device_id1,
            sizeof(config_cam->device_id1) - 1,
            IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID1_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK8_CONFIG_CAM_DEVICE_ID1_KEY,
            IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID1_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            IIDXHOOK8_CONFIG_CAM_DEVICE_ID2_KEY,
            config_cam->device_id2,
            sizeof(config_cam->device_id2) - 1,
            IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID2_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK8_CONFIG_CAM_DEVICE_ID2_KEY,
            IIDXHOOK8_CONFIG_CAM_DEFAULT_DEVICE_ID2_VALUE);
    }
}

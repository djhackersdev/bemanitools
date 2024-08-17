#include "camhook/config-cam.h"

#include "core/config-ext.h"

#include "iface-core/log.h"

#include "util/str.h"

void camhook_config_cam_get(
    const bt_core_config_t *config,
    camhook_config_cam_t *config_cam,
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
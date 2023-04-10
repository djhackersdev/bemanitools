#ifndef CAMHOOK_CONFIG_CAM_H
#define CAMHOOK_CONFIG_CAM_H

#include <windows.h>

#include "cconfig/cconfig.h"

#define CAMHOOK_CONFIG_CAM_MAX 2

struct camhook_config_cam {
    bool disable_emu;
    size_t num_devices;
    char device_id[CAMHOOK_CONFIG_CAM_MAX][MAX_PATH];
    bool disable_camera[CAMHOOK_CONFIG_CAM_MAX];
};

void camhook_config_cam_init(struct cconfig *config, size_t num_cams);

void camhook_config_cam_get(
    struct camhook_config_cam *config_cam,
    struct cconfig *config,
    size_t num_cams);

#endif
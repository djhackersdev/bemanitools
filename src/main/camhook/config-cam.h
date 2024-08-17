#ifndef CAMHOOK_CONFIG_CAM_H
#define CAMHOOK_CONFIG_CAM_H

#include <windows.h>

#include "iface-core/config.h"

#define CAMHOOK_CONFIG_CAM_MAX 2

typedef struct camhook_config_cam {
    bool disable_emu;
    size_t num_devices;
    int port_layout;
    char device_id[CAMHOOK_CONFIG_CAM_MAX][MAX_PATH];
    bool disable_camera[CAMHOOK_CONFIG_CAM_MAX];
} camhook_config_cam_t;

void camhook_config_cam_get(
    const bt_core_config_t *config,
    camhook_config_cam_t *config_cam,
    size_t num_cams,
    bool use_port_layout);

#endif
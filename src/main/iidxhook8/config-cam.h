#ifndef IIDXHOOK8_CONFIG_CAM_H
#define IIDXHOOK8_CONFIG_CAM_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct iidxhook8_config_cam {
    bool disable_emu;
    char device_id1[MAX_PATH];
    char device_id2[MAX_PATH];
};

void iidxhook8_config_cam_init(struct cconfig* config);

void iidxhook8_config_cam_get(struct iidxhook8_config_cam* config_cam, 
        struct cconfig* config);

#endif
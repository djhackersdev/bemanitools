#ifndef CAMHOOK_CAM_H
#define CAMHOOK_CAM_H

#include "camhook/config-cam.h"

enum camhook_version {
    CAMHOOK_OLD,
    CAMHOOK_NEW,
};

void camhook_set_version(enum camhook_version version);

void camhook_init(struct camhook_config_cam *config_cam);

void camhook_fini(void);

#endif

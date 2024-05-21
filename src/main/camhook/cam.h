#ifndef CAMHOOK_CAM_H
#define CAMHOOK_CAM_H

#include "camhook/config-cam.h"

// unused to control which camhook version is being used
// defaults to CAMHOOK_VERSION_OLD
enum camhook_version {
    CAMHOOK_VERSION_OLD,
    CAMHOOK_VERSION_NEW,
};

void camhook_set_version(enum camhook_version version);

void camhook_init(struct camhook_config_cam *config_cam);

void camhook_fini(void);

#endif

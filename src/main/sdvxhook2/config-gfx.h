#ifndef SDVXHOOK2_CONFIG_GFX_H
#define SDVXHOOK2_CONFIG_GFX_H

#include "cconfig/cconfig.h"

struct sdvxhook2_config_gfx {
    bool framed;
    uint16_t pci_id_vid;
    uint16_t pci_id_pid;
    bool windowed;
    int32_t window_width;
    int32_t window_height;
};

void sdvxhook2_config_gfx_init(struct cconfig *config);

void sdvxhook2_config_gfx_get(
    struct sdvxhook2_config_gfx *config_gfx, struct cconfig *config);

#endif
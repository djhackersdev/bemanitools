#ifndef IIDXHOOK_CONFIG_GFX_H
#define IIDXHOOK_CONFIG_GFX_H

#include "cconfig/cconfig.h"

#include "iidxhook-util/d3d9.h"

struct iidxhook_config_gfx {
    bool bgvideo_uv_fix;
    bool framed;
    float frame_rate_limit;
    float monitor_check;
    uint16_t pci_id_vid;
    uint16_t pci_id_pid;
    bool windowed;
    int32_t window_width;
    int32_t window_height;
    uint16_t scale_back_buffer_width;
    uint16_t scale_back_buffer_height;
    enum iidxhook_util_d3d9_back_buffer_scale_filter scale_back_buffer_filter;
};

void iidxhook_config_gfx_init(struct cconfig* config);

void iidxhook_config_gfx_get(struct iidxhook_config_gfx* config_gfx, 
        struct cconfig* config);

#endif
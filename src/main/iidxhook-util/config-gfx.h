#ifndef IIDXHOOK_CONFIG_GFX_H
#define IIDXHOOK_CONFIG_GFX_H

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

#include "iidxhook-util/d3d9.h"

// see struct iidxhook_util_d3d9_config for more info
typedef struct iidxhook_config_gfx {
    bool bgvideo_uv_fix;
    bool framed;
    float frame_rate_limit;
    float monitor_check;
    uint16_t pci_id_vid;
    uint16_t pci_id_pid;
    bool windowed;
    uint16_t window_width;
    uint16_t window_height;
    uint16_t scale_back_buffer_width;
    uint16_t scale_back_buffer_height;
    enum iidxhook_util_d3d9_back_buffer_scale_filter scale_back_buffer_filter;
    int16_t forced_refresh_rate;
    int8_t device_adapter;
    bool diagonal_tearing_fix;
    bool happy_sky_ms_bg_fix;
    bool distorted_ms_bg_fix;
} iidxhook_config_gfx_t;

void iidxhook_config_gfx_init(struct cconfig *config);

void iidxhook_config_gfx_get(
    struct iidxhook_config_gfx *config_gfx, struct cconfig *config);

void iidxhook_util_config_gfx_get2(
    const bt_core_config_t *config,
    iidxhook_config_gfx_t *config_gfx);

#endif
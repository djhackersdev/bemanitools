#include <math.h>
#include <string.h>

#include "iface-core/log.h"

#include "iidxhook-util/config-gfx.h"

#include "util/str.h"

static void _iidxhook_util_config_gfx_verify(const iidxhook_config_gfx_t *config)
{
    if (config->frame_rate_limit < 0.0 || config->frame_rate_limit > 500.0) {
        log_fatal("Invalid frame_rate_limit value in gfx configuration: %f", config->frame_rate_limit);
    }

    if (!(fabs(config->monitor_check - (-1.0)) < 0.001 ||
          fabs(config->monitor_check) < 0.001 || 
          config->monitor_check < 500.0)) {
        log_fatal("Invalid monitor_check value in gfx configuration: %f", config->monitor_check);
    }

    if (config->window_width > 10000 || config->window_width < 0) {
        log_fatal("Invalid window_width value in gfx configuration: %d", config->window_width);
    }

    if (config->window_height > 10000 || config->window_height < 0) {
        log_fatal("Invalid window_height value in gfx configuration: %d", config->window_height);
    }

    if (config->scale_back_buffer_width > 10000) {
        log_fatal("Invalid scale_back_buffer_width value in gfx configuration: %d", config->scale_back_buffer_width);
    }

    if (config->scale_back_buffer_height > 10000) {
        log_fatal("Invalid scale_back_buffer_height value in gfx configuration: %d", config->scale_back_buffer_height);
    }

    if (config->forced_refresh_rate < -1 || config->forced_refresh_rate > 500) {
        log_fatal("Invalid forced_refresh_rate value in gfx configuration: %d", config->forced_refresh_rate);
    }

    if (config->device_adapter < -1 || config->device_adapter > 32) {
        log_fatal("Invalid device_adapter value in gfx configuration: %d", config->device_adapter);
    }
}

static void _iidxhook_util_config_gfx_back_buffer_scale_filter_get(
    const bt_core_config_t *config,
    enum iidxhook_util_d3d9_back_buffer_scale_filter *filter)
{
    char tmp[16];

    bt_core_config_str_get(config, "gfx/back_buffer_scale/filter", tmp, sizeof(tmp));

    if (str_eq(tmp, "none")) {
        *filter = IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE;
    } else if (str_eq(tmp, "linear")) {
        *filter = IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR;
    } else if (str_eq(tmp, "point")) {
        *filter = IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_POINT;
    } else {
        log_fatal("Invalid value for back_buffer_scale/filter in gfx config: %s", tmp);
    }
}

void iidxhook_util_config_gfx_get(
    const bt_core_config_t *config,
    iidxhook_config_gfx_t *config_gfx)
{
    bt_core_config_s8_get(config, "gfx/device/adapter", &config_gfx->device_adapter);
    bt_core_config_s16_get(config, "gfx/device/forced_refresh_rate", &config_gfx->forced_refresh_rate);
    bt_core_config_float_get(config, "gfx/device/frame_rate_limit", &config_gfx->frame_rate_limit);
    bt_core_config_u16_get(config, "gfx/device/pci_id_vid", &config_gfx->pci_id_vid);
    bt_core_config_u16_get(config, "gfx/device/pci_id_pid", &config_gfx->pci_id_pid);

    bt_core_config_bool_get(config, "gfx/window/windowed", &config_gfx->windowed);
    bt_core_config_bool_get(config, "gfx/window/framed", &config_gfx->framed);
    bt_core_config_u16_get(config, "gfx/window/width", &config_gfx->window_width);
    bt_core_config_u16_get(config, "gfx/window/height", &config_gfx->window_height);

    bt_core_config_u16_get(config, "gfx/back_buffer_scale/width", &config_gfx->scale_back_buffer_width);
    bt_core_config_u16_get(config, "gfx/back_buffer_scale/height", &config_gfx->scale_back_buffer_height);
    _iidxhook_util_config_gfx_back_buffer_scale_filter_get(config, &config_gfx->scale_back_buffer_filter);

    bt_core_config_bool_get(config, "gfx/game/bgvideo_uv_fix", &config_gfx->bgvideo_uv_fix);
    bt_core_config_float_get(config, "gfx/game/monitor_check", &config_gfx->monitor_check);
    bt_core_config_bool_get(config, "gfx/game/diagonal_tearing_fix", &config_gfx->diagonal_tearing_fix);
    bt_core_config_bool_get(config, "gfx/game/happy_sky_ms_bg_fix", &config_gfx->happy_sky_ms_bg_fix);
    bt_core_config_bool_get(config, "gfx/game/distorted_ms_bg_fix", &config_gfx->distorted_ms_bg_fix);

    _iidxhook_util_config_gfx_verify(config_gfx);
}
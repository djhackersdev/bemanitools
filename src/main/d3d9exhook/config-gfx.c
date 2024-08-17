#include <d3d9.h>

#include <string.h>

#include "d3d9exhook/config-gfx.h"

#include "iface-core/log.h"

void d3d9exhook_config_gfx_get(
    const bt_core_config_t *config,
    d3d9exhook_config_gfx_t *config_gfx)
{
    bt_core_config_s32_get(config, "gfx/device/adapter", &config_gfx->device_adapter);
    bt_core_config_s32_get(config, "gfx/device/forced_refresh_rate", &config_gfx->forced_refresh_rate);
    bt_core_config_s32_get(config, "gfx/device/force_orientation", &config_gfx->force_orientation);
    bt_core_config_s32_get(config, "gfx/device/force_screen_res/width", &config_gfx->force_screen_res.width);
    bt_core_config_s32_get(config, "gfx/device/force_screen_res/height", &config_gfx->force_screen_res.height);

    bt_core_config_bool_get(config, "gfx/window/windowed", &config_gfx->windowed);
    bt_core_config_bool_get(config, "gfx/window/framed", &config_gfx->framed);
    bt_core_config_bool_get(config, "gfx/window/confined", &config_gfx->confined);
    bt_core_config_s32_get(config, "gfx/window/width", &config_gfx->window_width);
    bt_core_config_s32_get(config, "gfx/window/height", &config_gfx->window_height);
    bt_core_config_s32_get(config, "gfx/window/x", &config_gfx->window_x);
    bt_core_config_s32_get(config, "gfx/window/y", &config_gfx->window_y);
}
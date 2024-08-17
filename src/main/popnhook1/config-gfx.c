#include "core/config-ext.h"

#include "popnhook1/config-gfx.h"

#define POPNHOOK1_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define POPNHOOK1_CONFIG_GFX_FRAMED_KEY "gfx.framed"
#define POPNHOOK1_CONFIG_GFX_WINDOW_WIDTH_KEY "gfx.window_width"
#define POPNHOOK1_CONFIG_GFX_WINDOW_HEIGHT_KEY "gfx.window_height"

void popnhook1_config_gfx_get(
    const bt_core_config_t *config,
    popnhook1_config_gfx_t *out_config)
{
    bt_core_config_bool_get(config, "gfx/windowed", &out_config->windowed);
    bt_core_config_bool_get(config, "gfx/framed", &out_config->framed);
    bt_core_config_s32_get(config, "gfx/window_width", &out_config->window_width);
    bt_core_config_s32_get(config, "gfx/window_height", &out_config->window_height);
}

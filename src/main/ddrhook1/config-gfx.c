#include <string.h>

#include "core/config-ext.h"

#include "ddrhook1/config-gfx.h"

void ddrhook1_config_gfx_get(
    const bt_core_config_t *config,
    ddrhook1_config_gfx_t *config_gfx)
{
    bt_core_config_ext_bool_get(config, "gfx/windowed", &config_gfx->windowed);
}

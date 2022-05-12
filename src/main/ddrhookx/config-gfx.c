#include <string.h>

#include "cconfig/cconfig-util.h"

#include "ddrhookx/config-gfx.h"

#include "util/log.h"

#define DDRHOOKX_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"

#define DDRHOOKX_CONFIG_GFX_DEFAULT_WINDOWED_VALUE true

void ddrhookx_config_gfx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        DDRHOOKX_CONFIG_GFX_WINDOWED_KEY,
        DDRHOOKX_CONFIG_GFX_DEFAULT_WINDOWED_VALUE,
        "Run the game windowed");
}

void ddrhookx_config_gfx_get(
    struct ddrhookx_config_gfx *config_gfx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            DDRHOOKX_CONFIG_GFX_WINDOWED_KEY,
            &config_gfx->windowed,
            DDRHOOKX_CONFIG_GFX_DEFAULT_WINDOWED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRHOOKX_CONFIG_GFX_WINDOWED_KEY,
            DDRHOOKX_CONFIG_GFX_DEFAULT_WINDOWED_VALUE);
    }
}

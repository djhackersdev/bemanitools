#include <string.h>

#include "cconfig/cconfig-util.h"

#include "core/log.h"

#include "ddrhook1/config-gfx.h"

#define DDRHOOK1_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"

#define DDRHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE true

void ddrhook1_config_gfx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        DDRHOOK1_CONFIG_GFX_WINDOWED_KEY,
        DDRHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE,
        "Run the game windowed");
}

void ddrhook1_config_gfx_get(
    struct ddrhook1_config_gfx *config_gfx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            DDRHOOK1_CONFIG_GFX_WINDOWED_KEY,
            &config_gfx->windowed,
            DDRHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRHOOK1_CONFIG_GFX_WINDOWED_KEY,
            DDRHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE);
    }
}

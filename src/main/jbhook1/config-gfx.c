#include <string.h>

#include "cconfig/cconfig-util.h"

#include "jbhook1/config-gfx.h"

#include "util/log.h"

#define JBHOOK1_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define JBHOOK1_CONFIG_GFX_VERTICAL_KEY "gfx.vertical"

#define JBHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE false
#define JBHOOK1_CONFIG_GFX_DEFAULT_VERTICAL_VALUE true

void jbhook1_config_gfx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        JBHOOK1_CONFIG_GFX_WINDOWED_KEY,
        JBHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE,
        "Run the game windowed");
    cconfig_util_set_bool(
        config,
        JBHOOK1_CONFIG_GFX_VERTICAL_KEY,
        JBHOOK1_CONFIG_GFX_DEFAULT_VERTICAL_VALUE,
        "Adjust the rotation of the game window so it runs vertically");
}

void jbhook1_config_gfx_get(
    struct jbhook1_config_gfx *config_gfx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            JBHOOK1_CONFIG_GFX_WINDOWED_KEY,
            &config_gfx->windowed,
            JBHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            JBHOOK1_CONFIG_GFX_WINDOWED_KEY,
            JBHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE);
    }
    if (!cconfig_util_get_bool(
            config,
            JBHOOK1_CONFIG_GFX_VERTICAL_KEY,
            &config_gfx->vertical,
            JBHOOK1_CONFIG_GFX_DEFAULT_VERTICAL_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            JBHOOK1_CONFIG_GFX_VERTICAL_KEY,
            JBHOOK1_CONFIG_GFX_DEFAULT_VERTICAL_VALUE);
    }
}

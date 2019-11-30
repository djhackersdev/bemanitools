#include <string.h>

#include "cconfig/cconfig-util.h"

#include "sdvxhook2/config-gfx.h"

#include "util/log.h"

#define SDVXHOOK2_CONFIG_GFX_FRAMED_KEY "gfx.framed"
#define SDVXHOOK2_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define SDVXHOOK2_CONFIG_GFX_WINDOW_WIDTH_KEY "gfx.window_width"
#define SDVXHOOK2_CONFIG_GFX_WINDOW_HEIGHT_KEY "gfx.window_height"

#define SDVXHOOK2_CONFIG_GFX_DEFAULT_FRAMED_VALUE false
#define SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOWED_VALUE false
#define SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE -1
#define SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE -1

void sdvxhook2_config_gfx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        SDVXHOOK2_CONFIG_GFX_FRAMED_KEY,
        SDVXHOOK2_CONFIG_GFX_DEFAULT_FRAMED_VALUE,
        "Run the game in a framed window (requires windowed option)");

    cconfig_util_set_bool(
        config,
        SDVXHOOK2_CONFIG_GFX_WINDOWED_KEY,
        SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOWED_VALUE,
        "Run the game windowed");

    cconfig_util_set_int(
        config,
        SDVXHOOK2_CONFIG_GFX_WINDOW_WIDTH_KEY,
        SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE,
        "Windowed width, -1 for default size");

    cconfig_util_set_int(
        config,
        SDVXHOOK2_CONFIG_GFX_WINDOW_HEIGHT_KEY,
        SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE,
        "Windowed height, -1 for default size");
}

void sdvxhook2_config_gfx_get(
    struct sdvxhook2_config_gfx *config_gfx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            SDVXHOOK2_CONFIG_GFX_FRAMED_KEY,
            &config_gfx->framed,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_FRAMED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CONFIG_GFX_FRAMED_KEY,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_FRAMED_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            SDVXHOOK2_CONFIG_GFX_WINDOWED_KEY,
            &config_gfx->windowed,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOWED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CONFIG_GFX_WINDOWED_KEY,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOWED_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            SDVXHOOK2_CONFIG_GFX_WINDOW_WIDTH_KEY,
            &config_gfx->window_width,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CONFIG_GFX_WINDOW_WIDTH_KEY,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            SDVXHOOK2_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            &config_gfx->window_height,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE);
    }
}

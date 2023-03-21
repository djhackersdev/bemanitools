#include <string.h>

#include "cconfig/cconfig-util.h"

#include "popnhook1/config-gfx.h"

#include "util/log.h"

#define POPNHOOK1_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define POPNHOOK1_CONFIG_GFX_FRAMED_KEY "gfx.framed"
#define POPNHOOK1_CONFIG_GFX_WINDOW_WIDTH_KEY "gfx.window_width"
#define POPNHOOK1_CONFIG_GFX_WINDOW_HEIGHT_KEY "gfx.window_height"

#define POPNHOOK1_CONFIG_GFX_DEFAULT_FRAMED_VALUE false
#define POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE false
#define POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE -1
#define POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE -1

void popnhook1_config_gfx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        POPNHOOK1_CONFIG_GFX_WINDOWED_KEY,
        POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE,
        "Run the game windowed");

    cconfig_util_set_bool(
        config,
        POPNHOOK1_CONFIG_GFX_FRAMED_KEY,
        POPNHOOK1_CONFIG_GFX_DEFAULT_FRAMED_VALUE,
        "Run the game in a framed window (requires windowed option)");

    cconfig_util_set_int(
        config,
        POPNHOOK1_CONFIG_GFX_WINDOW_WIDTH_KEY,
        POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE,
        "Windowed width, -1 for default size");

    cconfig_util_set_int(
        config,
        POPNHOOK1_CONFIG_GFX_WINDOW_HEIGHT_KEY,
        POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE,
        "Windowed height, -1 for default size");
}

void popnhook1_config_gfx_get(
    struct popnhook1_config_gfx *config_gfx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            POPNHOOK1_CONFIG_GFX_WINDOWED_KEY,
            &config_gfx->windowed,
            POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            POPNHOOK1_CONFIG_GFX_WINDOWED_KEY,
            POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOWED_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            POPNHOOK1_CONFIG_GFX_FRAMED_KEY,
            &config_gfx->framed,
            POPNHOOK1_CONFIG_GFX_DEFAULT_FRAMED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            POPNHOOK1_CONFIG_GFX_FRAMED_KEY,
            POPNHOOK1_CONFIG_GFX_DEFAULT_FRAMED_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            POPNHOOK1_CONFIG_GFX_WINDOW_WIDTH_KEY,
            &config_gfx->window_width,
            POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            POPNHOOK1_CONFIG_GFX_WINDOW_WIDTH_KEY,
            POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            POPNHOOK1_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            &config_gfx->window_height,
            POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            POPNHOOK1_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            POPNHOOK1_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE);
    }
}

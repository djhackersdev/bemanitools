#include <d3d9.h>

#include <string.h>

#include "cconfig/cconfig-util.h"

#include "d3d9exhook/config-gfx.h"

#include "util/log.h"

#define D3D9EXHOOK_CONFIG_GFX_FRAMED_KEY "gfx.framed"
#define D3D9EXHOOK_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define D3D9EXHOOK_CONFIG_GFX_CONFINED_KEY "gfx.confined"
#define D3D9EXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY "gfx.window_width"
#define D3D9EXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY "gfx.window_height"
#define D3D9EXHOOK_CONFIG_GFX_WINDOW_X_KEY "gfx.window_x"
#define D3D9EXHOOK_CONFIG_GFX_WINDOW_Y_KEY "gfx.window_y"
#define D3D9EXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY "gfx.forced_refresh_rate"
#define D3D9EXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY "gfx.device_adapter"
#define D3D9EXHOOK_CONFIG_GFX_FORCE_ORIENTATION_KEY "gfx.force_orientation"
#define D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_WIDTH_KEY \
    "gfx.force_screen_res.width"
#define D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_HEIGHT_KEY \
    "gfx.force_screen_res.height"

#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE false
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE false
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_CONFINED_VALUE false
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_X_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_Y_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_ORIENTATION_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_WIDTH_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_HEIGHT_VALUE -1

void d3d9exhook_config_gfx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        D3D9EXHOOK_CONFIG_GFX_FRAMED_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE,
        "Run the game in a framed window (requires windowed option)");

    cconfig_util_set_bool(
        config,
        D3D9EXHOOK_CONFIG_GFX_WINDOWED_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE,
        "Run the game windowed");

    cconfig_util_set_bool(
        config,
        D3D9EXHOOK_CONFIG_GFX_CONFINED_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_CONFINED_VALUE,
        "Confine mouse cursor to window");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE,
        "Windowed width, -1 for default size");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE,
        "Windowed height, -1 for default size");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_WINDOW_X_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_X_VALUE,
        "Windowed X, -1 for default X position");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_WINDOW_Y_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_Y_VALUE,
        "Windowed Y, -1 for default Y position");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE,
        "Forced refresh rate, -1 to not force any (try 59 or 60 if monitor "
        "check fails to lock on high refresh rate monitors)");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE,
        "D3D9ex device adapter (monitor), -1 to use default, "
        "0, 1, 2 etc. to use specified adapter");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_FORCE_ORIENTATION_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_ORIENTATION_VALUE,
        "Orientation to force monitor into, -1 to use default, "
        "0, 1, 2, 3 to do 0, 90, 180, 270 degrees");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_WIDTH_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_WIDTH_VALUE,
        "Force a screen resolution (width), -1 to disable. Use this if the "
        "game does not auto "
        "detect your monitor's resolution properly, e.g. 1368x768 instead of "
        "1280x720.");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_HEIGHT_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_HEIGHT_VALUE,
        "Force a screen resolution (height), -1 to disable. Use this if the "
        "game does not auto "
        "detect your monitor's resolution properly, e.g. 1368x768 instead of "
        "1280x720.");
}

void d3d9exhook_config_gfx_get(
    struct d3d9exhook_config_gfx *config_gfx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            D3D9EXHOOK_CONFIG_GFX_FRAMED_KEY,
            &config_gfx->framed,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_FRAMED_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            D3D9EXHOOK_CONFIG_GFX_WINDOWED_KEY,
            &config_gfx->windowed,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_WINDOWED_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            D3D9EXHOOK_CONFIG_GFX_CONFINED_KEY,
            &config_gfx->confined,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_CONFINED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_CONFINED_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_CONFINED_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY,
            &config_gfx->window_width,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            &config_gfx->window_height,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_WINDOW_X_KEY,
            &config_gfx->window_x,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_X_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_WINDOW_X_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_X_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_WINDOW_Y_KEY,
            &config_gfx->window_y,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_Y_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_WINDOW_Y_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_Y_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY,
            &config_gfx->forced_refresh_rate,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY,
            &config_gfx->device_adapter,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_FORCE_ORIENTATION_KEY,
            &config_gfx->force_orientation,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_ORIENTATION_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_FORCE_ORIENTATION_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_ORIENTATION_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_WIDTH_KEY,
            &config_gfx->force_screen_res.width,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_WIDTH_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_WIDTH_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_WIDTH_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_HEIGHT_KEY,
            &config_gfx->force_screen_res.height,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_HEIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            D3D9EXHOOK_CONFIG_GFX_FORCE_SCREEN_RES_HEIGHT_KEY,
            D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCE_SCREEN_RES_HEIGHT_VALUE);
    }
}

#include <d3d9.h>

#include <string.h>

#include "cconfig/cconfig-util.h"

#include "d3d9exhook/config-gfx.h"

#include "util/log.h"

#define D3D9EXHOOK_CONFIG_GFX_FRAMED_KEY "gfx.framed"
#define D3D9EXHOOK_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define D3D9EXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY "gfx.window_width"
#define D3D9EXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY "gfx.window_height"
#define D3D9EXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY "gfx.forced_refresh_rate"
#define D3D9EXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY "gfx.device_adapter"

#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE false
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE false
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE -1
#define D3D9EXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE -1

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
        D3D9EXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE,
        "Forced refresh rate, -1 to not force any (try 59 or 60 if monitor "
        "check fails to lock on high refresh rate monitors)");

    cconfig_util_set_int(
        config,
        D3D9EXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY,
        D3D9EXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE,
        "D3D9ex device adapter (monitor), -1 to use default,"
        "0, 1, 2 etc. to use specified adapter");
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
}

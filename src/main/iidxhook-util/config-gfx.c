#include <string.h>

#include "cconfig/cconfig-util.h"

#include "iidxhook-util/config-gfx.h"

#include "util/log.h"

#define IIDXHOOK_CONFIG_GFX_BGVIDEO_UV_FIX_KEY "gfx.bgvideo_uv_fix"
#define IIDXHOOK_CONFIG_GFX_FRAMED_KEY "gfx.framed"
#define IIDXHOOK_CONFIG_GFX_FRAME_RATE_LIMIT_KEY "gfx.frame_rate_limit"
#define IIDXHOOK_CONFIG_GFX_MONITOR_CHECK_KEY "gfx.monitor_check"
#define IIDXHOOK_CONFIG_GFX_PCI_ID_KEY "gfx.pci_id"
#define IIDXHOOK_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define IIDXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY "gfx.window_width"
#define IIDXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY "gfx.window_height"
#define IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_WIDTH_KEY \
    "gfx.scale_back_buffer_width"
#define IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_HEIGHT_KEY \
    "gfx.scale_back_buffer_height"
#define IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_FILTER_KEY \
    "gfx.scale_back_buffer_filter"
#define IIDXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY "gfx.forced_refresh_rate"
#define IIDXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY "gfx.device_adapter"
#define IIDXHOOK_CONFIG_GFX_DIAGONAL_TEARING_FIX_KEY "gfx.diagonal_tearing_fix"

#define IIDXHOOK_CONFIG_GFX_DEFAULT_BGVIDEO_UV_FIX_VALUE false
#define IIDXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE false
#define IIDXHOOK_CONFIG_GFX_DEFAULT_FRAME_RATE_LIMIT_VALUE 0.0
#define IIDXHOOK_CONFIG_GFX_DEFAULT_MONITOR_CHECK_VALUE -1.0
#define IIDXHOOK_CONFIG_GFX_DEFAULT_PCI_ID_VALUE "1002:7146"
#define IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE false
#define IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE -1
#define IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE -1
#define IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_WIDTH_VALUE 0
#define IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_HEIGHT_VALUE 0
#define IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_FILTER_VALUE "none"
#define IIDXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE -1
#define IIDXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE -1
#define IIDXHOOK_CONFIG_GFX_DEFAULT_DIAGONAL_TEARING_FIX_VALUE false

void iidxhook_config_gfx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_GFX_BGVIDEO_UV_FIX_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_BGVIDEO_UV_FIX_VALUE,
        "Fix stretched BG videos on newer GPUs. Might appear on Red and "
        "newer");

    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_GFX_FRAMED_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE,
        "Run the game in a framed window (requires windowed option)");

    cconfig_util_set_float(
        config,
        IIDXHOOK_CONFIG_GFX_FRAME_RATE_LIMIT_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_FRAME_RATE_LIMIT_VALUE,
        "Software limit the frame rate of the rendering loop in hz, e.g. "
        "60 or 59.95 (0.0 = no software limit)");

    cconfig_util_set_float(
        config,
        IIDXHOOK_CONFIG_GFX_MONITOR_CHECK_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_MONITOR_CHECK_VALUE,
        "Enable/disable software monitor check/auto timebase or set "
        "a pre-determined refresh value. -1 disables this feature. 0 "
        "enables "
        "auto detecting the current refresh rate on startup. Setting any "
        "positive value > 0 allows you to set a pre-determined refresh "
        "rate "
        "(e.g. retrieved from the monitor check on newer IIDX games). "
        "Either "
        "the auto detected value or pre-determined value is used to patch "
        "any "
        "chart files in-memory to fix song synchronization issues. "
        "Requires "
        "constant refresh rate!!!");

    cconfig_util_set_str(
        config,
        IIDXHOOK_CONFIG_GFX_PCI_ID_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_PCI_ID_VALUE,
        "Patch the GPU device ID detection (leave empty to"
        " disable), format XXXX:YYYY, two 4 digit hex numbers (vid:pid)."
        " Examples: 1002:7146 (RV515, Radeon X1300), 1002:95C5 (RV620 LE,"
        " Radeon HD3450)");

    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_GFX_WINDOWED_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE,
        "Run the game windowed");

    cconfig_util_set_int(
        config,
        IIDXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE,
        "Windowed width, -1 for default size");

    cconfig_util_set_int(
        config,
        IIDXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE,
        "Windowed height, -1 for default size");

    cconfig_util_set_int(
        config,
        IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_WIDTH_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_WIDTH_VALUE,
        "Up-/downscale the back buffer's width. This does not change the "
        "game's rendering resolution but scales "
        "the "
        "final frame. Use this to target the native resolution of your "
        "monitor/TV, e.g. to avoid over-/underscan, "
        "bad "
        "image quality or latency caused by the monitors internal "
        "upscaler. 0 to disable this feature. Must be set "
        "in "
        "combination with the corresponding height parameter.");

    cconfig_util_set_int(
        config,
        IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_HEIGHT_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_HEIGHT_VALUE,
        "Up-/downscale the back buffer's height. This does not change the "
        "game's rendering resolution but scales "
        "the "
        "final frame. Use this to target the native resolution of your "
        "monitor/TV, e.g. to avoid over-/underscan, "
        "bad "
        "image quality or latency caused by the monitors internal "
        "upscaler. 0 to disable this feature. Must be set "
        "in "
        "combination with the corresponding width parameter.");

    cconfig_util_set_str(
        config,
        IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_FILTER_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_FILTER_VALUE,
        "Filter type to use for up-/downscaling the back buffer. Only used "
        "if scaling feature was enabled by "
        "setting "
        "the scaling width and height parameters. Available types: none, "
        "linear, point (refer to "
        "D3DTEXTUREFILTERTYPE "
        " for explanation).");

    cconfig_util_set_int(
        config,
        IIDXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE,
        "Forced refresh rate, -1 to not force any (try 59 or 60 if monitor "
        "check fails to lock on high refresh rate monitors)");

    cconfig_util_set_int(
        config,
        IIDXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE,
        "D3D9 device adapter (monitor), -1 to use default, "
        "0, 1, 2 etc. to use specified adapter");
    
    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_GFX_DIAGONAL_TEARING_FIX_KEY,
        IIDXHOOK_CONFIG_GFX_DEFAULT_DIAGONAL_TEARING_FIX_VALUE,
        "Fix diagonal tearing with video cards "
        "other than Radeon X1300 and HD3450");
}

void iidxhook_config_gfx_get(
    struct iidxhook_config_gfx *config_gfx, struct cconfig *config)
{
    char tmp[10];
    char *vid;
    char *pid;
    int32_t tmp_int;

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_GFX_BGVIDEO_UV_FIX_KEY,
            &config_gfx->bgvideo_uv_fix,
            IIDXHOOK_CONFIG_GFX_DEFAULT_BGVIDEO_UV_FIX_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_BGVIDEO_UV_FIX_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_BGVIDEO_UV_FIX_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_GFX_FRAMED_KEY,
            &config_gfx->framed,
            IIDXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_FRAMED_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_FRAMED_VALUE);
    }

    if (!cconfig_util_get_float(
            config,
            IIDXHOOK_CONFIG_GFX_FRAME_RATE_LIMIT_KEY,
            &config_gfx->frame_rate_limit,
            IIDXHOOK_CONFIG_GFX_DEFAULT_FRAME_RATE_LIMIT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%f'",
            IIDXHOOK_CONFIG_GFX_FRAME_RATE_LIMIT_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_FRAME_RATE_LIMIT_VALUE);
    }

    if (!cconfig_util_get_float(
            config,
            IIDXHOOK_CONFIG_GFX_MONITOR_CHECK_KEY,
            &config_gfx->monitor_check,
            IIDXHOOK_CONFIG_GFX_DEFAULT_MONITOR_CHECK_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%f'",
            IIDXHOOK_CONFIG_GFX_MONITOR_CHECK_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_MONITOR_CHECK_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            IIDXHOOK_CONFIG_GFX_PCI_ID_KEY,
            tmp,
            sizeof(tmp) - 1,
            IIDXHOOK_CONFIG_GFX_DEFAULT_PCI_ID_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK_CONFIG_GFX_PCI_ID_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_PCI_ID_VALUE);
    }

    if (tmp[4] != ':') {
        log_warning(
            "Invalid format for value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK_CONFIG_GFX_PCI_ID_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_PCI_ID_VALUE);
        strcpy(tmp, IIDXHOOK_CONFIG_GFX_DEFAULT_PCI_ID_VALUE);
    }

    tmp[4] = '\0';
    vid = tmp;
    pid = &tmp[5];
    config_gfx->pci_id_vid = strtol(vid, NULL, 16);
    config_gfx->pci_id_pid = strtol(pid, NULL, 16);

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_GFX_WINDOWED_KEY,
            &config_gfx->windowed,
            IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_WINDOWED_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOWED_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            IIDXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY,
            &config_gfx->window_width,
            IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_WINDOW_WIDTH_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_WIDTH_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            IIDXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            &config_gfx->window_height,
            IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_WINDOW_HEIGHT_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_WINDOW_HEIGHT_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_WIDTH_KEY,
            &tmp_int,
            IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_WIDTH_VALUE) ||
        tmp_int < 0 || tmp_int > 0xFFFF) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_WIDTH_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_WIDTH_VALUE);
    }

    config_gfx->scale_back_buffer_width = (uint16_t) tmp_int;

    if (!cconfig_util_get_int(
            config,
            IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_HEIGHT_KEY,
            &tmp_int,
            IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_HEIGHT_VALUE) ||
        tmp_int < 0 || tmp_int > 0xFFFF) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_HEIGHT_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_HEIGHT_VALUE);
    }

    config_gfx->scale_back_buffer_height = (uint16_t) tmp_int;

    if (!cconfig_util_get_str(
            config,
            IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_FILTER_KEY,
            tmp,
            sizeof(tmp) - 1,
            IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_FILTER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_FILTER_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_FILTER_VALUE);
    }

    if (!strcmp(tmp, "none")) {
        config_gfx->scale_back_buffer_filter =
            IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE;
    } else if (!strcmp(tmp, "linear")) {
        config_gfx->scale_back_buffer_filter =
            IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR;
    } else if (!strcmp(tmp, "point")) {
        config_gfx->scale_back_buffer_filter =
            IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_POINT;
    } else {
        config_gfx->scale_back_buffer_filter =
            IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE;

        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            IIDXHOOK_CONFIG_GFX_SCALE_BACK_BUFFER_FILTER_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_SCALE_BACK_BUFFER_FILTER_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            IIDXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY,
            &config_gfx->forced_refresh_rate,
            IIDXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_FORCED_REFRESHRATE_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_FORCED_RR_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            IIDXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY,
            &config_gfx->device_adapter,
            IIDXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_DEVICE_ADAPTER_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_DEVICE_ADAPTER_VALUE);
    }

    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_GFX_DIAGONAL_TEARING_FIX_KEY,
            &config_gfx->diagonal_tearing_fix,
            IIDXHOOK_CONFIG_GFX_DEFAULT_DIAGONAL_TEARING_FIX_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_GFX_DIAGONAL_TEARING_FIX_KEY,
            IIDXHOOK_CONFIG_GFX_DEFAULT_DIAGONAL_TEARING_FIX_VALUE);
    }
}

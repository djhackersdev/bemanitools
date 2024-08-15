#include <math.h>
#include <string.h>

#include "cconfig/cconfig-util.h"

#include "iface-core/log.h"

#include "iidxhook-util/config-gfx.h"

#include "util/str.h"

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
        "Fix stretched BG videos on newer GPUs. Might appear on SIRIUS "
        "and older.  On 9th and 10th style this issue may only affect "
        "older BGAs (from 1st-3rd style)");

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
            (int32_t*) &config_gfx->window_width,
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
            (int32_t*) &config_gfx->window_height,
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
            (int32_t*) &config_gfx->forced_refresh_rate,
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
            (int32_t*) &config_gfx->device_adapter,
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

static void _iidxhook_util_config_gfx_verify(const iidxhook_config_gfx_t *config)
{
    if (config->frame_rate_limit < 0.0 || config->frame_rate_limit > 500.0) {
        log_fatal("Invalid frame_rate_limit value in gfx configuration: %f", config->frame_rate_limit);
    }

    if (!(fabs(config->monitor_check - (-1.0)) < 0.001 ||
          fabs(config->monitor_check) < 0.001 || 
          config->monitor_check < 500.0)) {
        log_fatal("Invalid monitor_check value in gfx configuration: %f", config->monitor_check);
    }

    if (config->window_width > 10000 || config->window_width < 0) {
        log_fatal("Invalid window_width value in gfx configuration: %d", config->window_width);
    }

    if (config->window_height > 10000 || config->window_height < 0) {
        log_fatal("Invalid window_height value in gfx configuration: %d", config->window_height);
    }

    if (config->scale_back_buffer_width > 10000) {
        log_fatal("Invalid scale_back_buffer_width value in gfx configuration: %d", config->scale_back_buffer_width);
    }

    if (config->scale_back_buffer_height > 10000) {
        log_fatal("Invalid scale_back_buffer_height value in gfx configuration: %d", config->scale_back_buffer_height);
    }

    if (config->forced_refresh_rate < -1 || config->forced_refresh_rate > 500) {
        log_fatal("Invalid forced_refresh_rate value in gfx configuration: %d", config->forced_refresh_rate);
    }

    if (config->device_adapter < -1 || config->device_adapter > 32) {
        log_fatal("Invalid device_adapter value in gfx configuration: %d", config->device_adapter);
    }
}

static void _iidxhook_util_config_gfx_back_buffer_scale_filter_get(
    const bt_core_config_t *config,
    enum iidxhook_util_d3d9_back_buffer_scale_filter *filter)
{
    char tmp[16];

    bt_core_config_str_get(config, "gfx/back_buffer_scale/filter", tmp, sizeof(tmp));

    if (str_eq(tmp, "none")) {
        *filter = IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_NONE;
    } else if (str_eq(tmp, "linear")) {
        *filter = IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_LINEAR;
    } else if (str_eq(tmp, "point")) {
        *filter = IIDXHOOK_UTIL_D3D9_BACK_BUFFER_SCALE_FILTER_POINT;
    } else {
        log_fatal("Invalid value for back_buffer_scale/filter in gfx config: %s", tmp);
    }
}

void iidxhook_util_config_gfx_get2(
    const bt_core_config_t *config,
    iidxhook_config_gfx_t *config_gfx)
{
    bt_core_config_s8_get(config, "gfx/device/adapter", &config_gfx->device_adapter);
    bt_core_config_s16_get(config, "gfx/device/forced_refresh_rate", &config_gfx->forced_refresh_rate);
    bt_core_config_float_get(config, "gfx/device/frame_rate_limit", &config_gfx->frame_rate_limit);
    bt_core_config_u16_get(config, "gfx/device/pci_id_vid", &config_gfx->pci_id_vid);
    bt_core_config_u16_get(config, "gfx/device/pci_id_pid", &config_gfx->pci_id_pid);

    bt_core_config_bool_get(config, "gfx/window/windowed", &config_gfx->windowed);
    bt_core_config_bool_get(config, "gfx/window/framed", &config_gfx->framed);
    bt_core_config_u16_get(config, "gfx/window/width", &config_gfx->window_width);
    bt_core_config_u16_get(config, "gfx/window/height", &config_gfx->window_height);

    bt_core_config_u16_get(config, "gfx/back_buffer_scale/width", &config_gfx->scale_back_buffer_width);
    bt_core_config_u16_get(config, "gfx/back_buffer_scale/height", &config_gfx->scale_back_buffer_height);
    _iidxhook_util_config_gfx_back_buffer_scale_filter_get(config, &config_gfx->scale_back_buffer_filter);

    bt_core_config_bool_get(config, "gfx/game/bgvideo_uv_fix", &config_gfx->bgvideo_uv_fix);
    bt_core_config_float_get(config, "gfx/game/monitor_check", &config_gfx->monitor_check);
    bt_core_config_bool_get(config, "gfx/game/diagonal_tearing_fix", &config_gfx->diagonal_tearing_fix);
    bt_core_config_bool_get(config, "gfx/game/happy_sky_ms_bg_fix", &config_gfx->happy_sky_ms_bg_fix);

    _iidxhook_util_config_gfx_verify(config_gfx);
}
#include <string.h>

#include "cconfig/cconfig-util.h"

#include "sdvxhook2/config-gfx.h"

#include "util/log.h"

#define SDVXHOOK2_CONFIG_GFX_FRAMED_KEY "gfx.framed"
#define SDVXHOOK2_CONFIG_GFX_PCI_ID_KEY "gfx.pci_id"
#define SDVXHOOK2_CONFIG_GFX_WINDOWED_KEY "gfx.windowed"
#define SDVXHOOK2_CONFIG_GFX_WINDOW_WIDTH_KEY "gfx.window_width"
#define SDVXHOOK2_CONFIG_GFX_WINDOW_HEIGHT_KEY "gfx.window_height"

#define SDVXHOOK2_CONFIG_GFX_DEFAULT_FRAMED_VALUE false
#define SDVXHOOK2_CONFIG_GFX_DEFAULT_PCI_ID_VALUE "1002:7146"
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

    cconfig_util_set_str(
        config,
        SDVXHOOK2_CONFIG_GFX_PCI_ID_KEY,
        SDVXHOOK2_CONFIG_GFX_DEFAULT_PCI_ID_VALUE,
        "Patch the GPU device ID detection (leave empty to"
        " disable), format XXXX:YYYY, two 4 digit hex numbers (vid:pid)."
        " Examples: 1002:7146 (RV515, Radeon X1300), 1002:95C5 (RV620 LE,"
        " Radeon HD3450)");

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
    char tmp[10];
    char *vid;
    char *pid;

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

    if (!cconfig_util_get_str(
            config,
            SDVXHOOK2_CONFIG_GFX_PCI_ID_KEY,
            tmp,
            sizeof(tmp) - 1,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_PCI_ID_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            SDVXHOOK2_CONFIG_GFX_PCI_ID_KEY,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_PCI_ID_VALUE);
    }

    if (tmp[4] != ':') {
        log_warning(
            "Invalid format for value for key '%s' specified, fallback "
            "to default '%s'",
            SDVXHOOK2_CONFIG_GFX_PCI_ID_KEY,
            SDVXHOOK2_CONFIG_GFX_DEFAULT_PCI_ID_VALUE);
        strcpy(tmp, SDVXHOOK2_CONFIG_GFX_DEFAULT_PCI_ID_VALUE);
    }

    tmp[4] = '\0';
    vid = tmp;
    pid = &tmp[5];
    config_gfx->pci_id_vid = strtol(vid, NULL, 16);
    config_gfx->pci_id_pid = strtol(pid, NULL, 16);

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

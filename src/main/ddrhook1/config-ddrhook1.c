#include <string.h>

#include "cconfig/cconfig-util.h"

#include "ddrhook1/config-ddrhook1.h"

#include "util/log.h"

#define DDRHOOK1_CONFIG_DDRHOOK1_USE_COM4_EMU_KEY "ddrhookx.use_com4_emu"
#define DDRHOOK1_CONFIG_DDRHOOK1_STANDARD_DEF_KEY "ddrhookx.standard_def"
#define DDRHOOK1_CONFIG_DDRHOOK1_USE_15KHZ_KEY "ddrhookx.use_15khz"
#define DDRHOOK1_CONFIG_DDRHOOK1_USBMEM_PATH "ddrhookx.usbmem_path"

#define DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_COM4_EMU_VALUE true
#define DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_STANDARD_DEF_VALUE false
#define DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_15KHZ_VALUE false
#define DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USBMEM_PATH "usbmem"

void ddrhook1_config_ddrhook1_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        DDRHOOK1_CONFIG_DDRHOOK1_USE_COM4_EMU_KEY,
        DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_COM4_EMU_VALUE,
        "Don't emulate P3IO COM4 and its downstream devices, use the Windows COM4 port instead");
    cconfig_util_set_bool(
        config,
        DDRHOOK1_CONFIG_DDRHOOK1_STANDARD_DEF_KEY,
        DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_STANDARD_DEF_VALUE,
        "SD cabinet mode");
    cconfig_util_set_bool(
        config,
        DDRHOOK1_CONFIG_DDRHOOK1_USE_15KHZ_KEY,
        DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_15KHZ_VALUE,
        "Use 15 kHz monitor mode");
    cconfig_util_set_str(
        config,
        DDRHOOK1_CONFIG_DDRHOOK1_USBMEM_PATH,
        DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USBMEM_PATH,
        "Specify path for USB memory data");
}

void ddrhook1_config_ddrhook1_get(
    struct ddrhook1_config_ddrhookx *config_ddrhookx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            DDRHOOK1_CONFIG_DDRHOOK1_USE_COM4_EMU_KEY,
            &config_ddrhookx->use_com4_emu,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_COM4_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRHOOK1_CONFIG_DDRHOOK1_USE_COM4_EMU_KEY,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_COM4_EMU_VALUE);
    }
    if (!cconfig_util_get_bool(
            config,
            DDRHOOK1_CONFIG_DDRHOOK1_STANDARD_DEF_KEY,
            &config_ddrhookx->standard_def,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_STANDARD_DEF_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRHOOK1_CONFIG_DDRHOOK1_STANDARD_DEF_KEY,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_STANDARD_DEF_VALUE);
    }
    if (!cconfig_util_get_bool(
            config,
            DDRHOOK1_CONFIG_DDRHOOK1_USE_15KHZ_KEY,
            &config_ddrhookx->standard_def,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_15KHZ_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRHOOK1_CONFIG_DDRHOOK1_USE_15KHZ_KEY,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USE_15KHZ_VALUE);
    }
    if (!cconfig_util_get_str(
            config,
            DDRHOOK1_CONFIG_DDRHOOK1_USBMEM_PATH,
            config_ddrhookx->usbmem_path,
            sizeof(config_ddrhookx->usbmem_path) - 1,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USBMEM_PATH)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            DDRHOOK1_CONFIG_DDRHOOK1_USBMEM_PATH,
            DDRHOOK1_CONFIG_DDRHOOK1_DEFAULT_USBMEM_PATH);
    }
}

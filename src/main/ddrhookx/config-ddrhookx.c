#include <string.h>

#include "cconfig/cconfig-util.h"

#include "ddrhookx/config-ddrhookx.h"

#include "util/log.h"

#define DDRHOOKX_CONFIG_DDRHOOKX_USE_COM4_EMU_KEY "ddrhookx.use_com4_emu"
#define DDRHOOKX_CONFIG_DDRHOOKX_STANDARD_DEF_KEY "ddrhookx.standard_def"
#define DDRHOOKX_CONFIG_DDRHOOKX_USBMEM_PATH "ddrhookx.usbmem_path"

#define DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USE_COM4_EMU_VALUE true
#define DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_STANDARD_DEF_VALUE false
#define DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USBMEM_PATH "usbmem"

void ddrhookx_config_ddrhookx_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        DDRHOOKX_CONFIG_DDRHOOKX_USE_COM4_EMU_KEY,
        DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USE_COM4_EMU_VALUE,
        "Don't emulate P3IO COM4 and its downstream devices, use the Windows COM4 port instead");
    cconfig_util_set_bool(
        config,
        DDRHOOKX_CONFIG_DDRHOOKX_STANDARD_DEF_KEY,
        DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_STANDARD_DEF_VALUE,
        "SD cabinet mode");
    cconfig_util_set_str(
        config,
        DDRHOOKX_CONFIG_DDRHOOKX_USBMEM_PATH,
        DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USBMEM_PATH,
        "Specify path for USB memory data");
}

void ddrhookx_config_ddrhookx_get(
    struct ddrhookx_config_ddrhookx *config_ddrhookx, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            DDRHOOKX_CONFIG_DDRHOOKX_USE_COM4_EMU_KEY,
            &config_ddrhookx->use_com4_emu,
            DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USE_COM4_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRHOOKX_CONFIG_DDRHOOKX_USE_COM4_EMU_KEY,
            DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USE_COM4_EMU_VALUE);
    }
    if (!cconfig_util_get_bool(
            config,
            DDRHOOKX_CONFIG_DDRHOOKX_STANDARD_DEF_KEY,
            &config_ddrhookx->standard_def,
            DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_STANDARD_DEF_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRHOOKX_CONFIG_DDRHOOKX_STANDARD_DEF_KEY,
            DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_STANDARD_DEF_VALUE);
    }
    if (!cconfig_util_get_str(
            config,
            DDRHOOKX_CONFIG_DDRHOOKX_USBMEM_PATH,
            config_ddrhookx->usbmem_path,
            sizeof(config_ddrhookx->usbmem_path) - 1,
            DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USBMEM_PATH)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            DDRHOOKX_CONFIG_DDRHOOKX_USBMEM_PATH,
            DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USBMEM_PATH);
    }
}

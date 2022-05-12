#include <string.h>

#include "cconfig/cconfig-util.h"

#include "ddrhookx/config-ddrhookx.h"

#include "util/log.h"

#define DDRHOOKX_CONFIG_DDRHOOKX_USE_COM4_EMU_KEY "ddrhookx.use_com4_emu"
#define DDRHOOKX_CONFIG_DDRHOOKX_STANDARD_DEF_KEY "ddrhookx.standard_def"

#define DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_USE_COM4_EMU_VALUE true
#define DDRHOOKX_CONFIG_DDRHOOKX_DEFAULT_STANDARD_DEF_VALUE false

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
}

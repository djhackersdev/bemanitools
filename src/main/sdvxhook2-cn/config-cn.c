#include "cconfig/cconfig-util.h"

#include "sdvxhook2-cn/config-cn.h"

#include "util/log.h"

#define SDVXHOOK2_CN_CONFIG_DISABLE_IO_EMU_KEY "io.disable_io_emu"
#define SDVXHOOK2_CN_CONFIG_UNIS_PATH_KEY "cn.unis_path"

#define SDVXHOOK2_CN_CONFIG_DEFAULT_DISABLE_IO_EMU_VALUE false
#define SDVXHOOK2_CN_CONFIG_DEFAULT_UNIS_PATH_VALUE "prop/unis.xml"

void sdvxhook2_cn_config_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        SDVXHOOK2_CN_CONFIG_DISABLE_IO_EMU_KEY,
        SDVXHOOK2_CN_CONFIG_DEFAULT_DISABLE_IO_EMU_VALUE,
        "Disable IO emulation and enable usage of real KFCA hardware");

    cconfig_util_set_str(
        config,
        SDVXHOOK2_CN_CONFIG_UNIS_PATH_KEY,
        SDVXHOOK2_CN_CONFIG_DEFAULT_UNIS_PATH_VALUE,
        "Path to unis version file under the prop.s folder");
}

void sdvxhook2_cn_config_get(
    struct sdvxhook2_cn_config *cn_config, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            SDVXHOOK2_CN_CONFIG_DISABLE_IO_EMU_KEY,
            &cn_config->disable_io_emu,
            SDVXHOOK2_CN_CONFIG_DEFAULT_DISABLE_IO_EMU_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXHOOK2_CN_CONFIG_DISABLE_IO_EMU_KEY,
            SDVXHOOK2_CN_CONFIG_DEFAULT_DISABLE_IO_EMU_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            SDVXHOOK2_CN_CONFIG_UNIS_PATH_KEY,
            cn_config->unis_path,
            sizeof(cn_config->unis_path),
            SDVXHOOK2_CN_CONFIG_DEFAULT_UNIS_PATH_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            SDVXHOOK2_CN_CONFIG_UNIS_PATH_KEY,
            SDVXHOOK2_CN_CONFIG_DEFAULT_UNIS_PATH_VALUE);
    }
}

#include "bio2drv/config.h"

#include "core/config-ext.h"

void bio2drv_config_bio2_get(
    const bt_core_config_t *config, bio2drv_config_t *config_out)
{
    bt_core_config_ext_bool_get(config, "autodetect", &config_out->autodetect);
    bt_core_config_ext_str_get(config, "port", config_out->port, sizeof(config_out->port));
    bt_core_config_ext_s32_get(config, "baud", &config_out->baud);
}
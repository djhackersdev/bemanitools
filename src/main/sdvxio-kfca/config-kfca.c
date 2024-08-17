#include "core/config-ext.h"

#include "sdvxio-kfca/config-kfca.h"

void sdvxio_kfca_config_kfca_get(
    const bt_core_config_t *config,
    sdvxio_kfca_config_kfca_t *config_out)
{
    bt_core_config_str_get(config, "kfca/port", config_out->port, sizeof(config_out->port));
    bt_core_config_s32_get(config, "kfca/baud", &config_out->baud);
    bt_core_config_s32_get(config, "kfca/main_override", &config_out->override_main_volume);
    bt_core_config_s32_get(config, "kfca/headphone_override", &config_out->override_headphone_volume);
    bt_core_config_s32_get(config, "kfca/subwoofer_override", &config_out->override_sub_volume);
}
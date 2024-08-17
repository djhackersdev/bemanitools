#include "core/config-ext.h"

#include "vigem-sdvxio/config.h"

void vigem_sdvxio_config_get(
    const bt_core_config_t *config,
    vigem_sdvxio_config_t *config_out)
{
    bt_core_config_ext_bool_get(config, "enable_keylight", &config_out->enable_keylight);
    bt_core_config_ext_bool_get(config, "use_relative_analog", &config_out->relative_analog);
    bt_core_config_ext_s32_get(config, "pwm_wings", &config_out->pwm_wings);
    bt_core_config_ext_s32_get(config, "pwm_controller", &config_out->pwm_controller);
    bt_core_config_ext_s32_get(config, "amp_volume", &config_out->amp_volume);
}
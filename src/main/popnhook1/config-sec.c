#include "core/config-ext.h"

#include "popnhook1/config-sec.h"

void popnhook1_config_sec_get(
    const bt_core_config_t *config,
    popnhook1_config_sec_t *out_config)
{
    bt_core_config_ext_security_mcode_get(config, "sec/black_plug_mcode", &out_config->black_plug_mcode);
}
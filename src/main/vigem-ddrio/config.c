#include "core/config-ext.h"

#include "vigem-ddrio/config.h"

void vigem_ddrio_config_get(const bt_core_config_t *config, vigem_ddrio_config_t *config_out)
{
    bt_core_config_ext_bool_get(config, "enable_reactive_light", &config_out->enable_reactive_light);
}

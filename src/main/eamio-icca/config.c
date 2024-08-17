#include "core/config-ext.h"

#include "eamio-icca/config.h"

void eamio_icca_config_icc_get(
    const bt_core_config_t *config, icc_config_t *config_out)
{
    bt_core_config_str_get(config, "port", config_out->port, sizeof(config_out->port));
    bt_core_config_s32_get(config, "baud", &config_out->baud);
}
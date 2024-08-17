#include "core/config-ext.h"

#include "jbio-p4io/config-h44b.h"

void jbio_config_h44b_get(
    const bt_core_config_t *config,
    h44b_config_t *config_out)
{
    bt_core_config_str_get(config, "h44b/port", config_out->port, sizeof(config_out->port));
    bt_core_config_s32_get(config, "h44b/baud", &config_out->baud);
}

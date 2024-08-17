#include "core/config-ext.h"

#include "popnhook1/config-eamuse.h"

void popnhook1_config_eamuse_get(
    const bt_core_config_t *config,
    popnhook1_config_eamuse_t *config_out)
{
    bt_core_config_ext_net_addr_get(config, "eamuse/server", &config_out->server);
    bt_core_config_ext_security_id_get(config, "eamuse/pcbid", &config_out->pcbid);
    bt_core_config_ext_security_id_get(config, "eamuse/eamid", &config_out->eamid);
}
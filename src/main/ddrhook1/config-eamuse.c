#include <string.h>
#include "core/config-ext.h"

#include "ddrhook1/config-eamuse.h"

void ddrhook1_config_eamuse_get(
    const bt_core_config_t *config,
    ddrhook1_config_eamuse_t *config_eamuse)
{
    bt_core_config_ext_net_addr_get(config, "eamuse/server", &config_eamuse->server);
    bt_core_config_ext_security_id_get(config, "eamuse/pcbid", &config_eamuse->pcbid);
    bt_core_config_ext_security_id_get(config, "eamuse/eamid", &config_eamuse->eamid);
}

#include "core/config-ext.h"

#include "ddrhook1/config-security.h"

void ddrhook1_config_security_get(
    const bt_core_config_t *config,
    ddrhook1_config_security_t *config_security)
{
    bt_core_config_ext_security_mcode_get(config, "security/mcode", &config_security->mcode);
}
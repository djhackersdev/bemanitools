#include "core/config-ext.h"

#include "jbhook1/config-security.h"

void jbhook1_config_security_get(
    const bt_core_config_t *config,
    jbhook1_config_security_t *config_out)
{
    bt_core_config_ext_security_mcode_get(config, "security/mcode", &config_out->mcode);
}
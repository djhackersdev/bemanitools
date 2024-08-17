#include <string.h>

#include "core/config-ext.h"

#include "iidxhook-util/config-eamuse.h"

#include "iface-core/log.h"

#include "security/mcode.h"

#include "util/str.h"

static void _iidxhook_util_config_eamuse_verify(
    const iidxhook_util_config_eamuse_t *config)
{
    char *tmp;

    if (!str_eq(config->card_type, "C02") &&
        !str_eq(config->card_type, "D01") &&
        !str_eq(config->card_type, "E11") &&
        !str_eq(config->card_type, "ECO")) {
      log_fatal("Invalid card_type in eamuse configuration: %s", config->card_type);
    }

    if (!security_id_verify(&config->pcbid)) {
        tmp = security_id_to_str(&config->pcbid, false);
        log_fatal("Invalid pcbid in eamuse configuration: %s", tmp);
        free(tmp);
    }

    if (!security_id_verify(&config->eamid)) {
        tmp = security_id_to_str(&config->eamid, false);
        log_fatal("Invalid eamid in eamuse configuration: %s", tmp);
        free(tmp);
    }
}

void iidxhook_util_config_eamuse_get(
    const bt_core_config_t *config,
    iidxhook_util_config_eamuse_t *config_eamuse)
{
    bt_core_config_ext_str_get(config, "eamuse/card_type", config_eamuse->card_type, sizeof(config_eamuse->card_type));
    bt_core_config_ext_net_addr_get(config, "eamuse/server", &config_eamuse->server);
    bt_core_config_ext_bin_get(config, "eamuse/pcbid", 
        (uint8_t *) &config_eamuse->pcbid, sizeof(config_eamuse->pcbid));
    bt_core_config_ext_bin_get(config, "eamuse/eamid", 
        (uint8_t *) &config_eamuse->eamid, sizeof(config_eamuse->eamid));

    _iidxhook_util_config_eamuse_verify(config_eamuse);
}

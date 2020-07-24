#include <string.h>

#include "imports/avs.h"

#include "launcher/ea3-config.h"
#include "launcher/module.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/str.h"

PSMAP_BEGIN(ea3_ident_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct ea3_ident, softid, "/ea3/id/softid", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct ea3_ident, hardid, "/ea3/id/hardid", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct ea3_ident, pcbid, "/ea3/id/pcbid", "")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, model, "/ea3/soft/model")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, dest, "/ea3/soft/dest")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, spec, "/ea3/soft/spec")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, rev, "/ea3/soft/rev")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, ext, "/ea3/soft/ext")
PSMAP_END

void ea3_ident_init(struct ea3_ident *ident)
{
    memset(ident, 0, sizeof(*ident));
}

bool ea3_ident_from_property(
    struct ea3_ident *ident, struct property *ea3_config)
{
    return property_psmap_import(ea3_config, NULL, ident, ea3_ident_psmap);
}

void ea3_ident_hardid_from_ethernet(struct ea3_ident *ident)
{
    struct avs_net_interface netif;
    int result;

    result = avs_net_ctrl(1, &netif, sizeof(netif));

    if (result < 0) {
        log_fatal(
            "avs_net_ctrl call to get MAC address returned error: %d", result);
    }

    ident->hardid[0] = '0';
    ident->hardid[1] = '1';
    ident->hardid[2] = '0';
    ident->hardid[3] = '0';

    hex_encode_uc(
        netif.mac_addr,
        sizeof(netif.mac_addr),
        ident->hardid + 4,
        sizeof(ident->hardid) - 4);
}

bool ea3_ident_invoke_module_init(
    struct ea3_ident *ident,
    const struct module_context *module,
    struct property_node *app_config)
{
    char sidcode_short[17];
    char sidcode_long[21];
    char security_code[9];
    bool ok;

    /* Set up security env vars */

    str_format(
        security_code,
        lengthof(security_code),
        "G*%s%s%s%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev);

    std_setenv("/env/boot/version", "0.0.0");
    std_setenv("/env/profile/security_code", security_code);
    std_setenv("/env/profile/system_id", ident->pcbid);
    std_setenv("/env/profile/account_id", ident->pcbid);
    std_setenv("/env/profile/license_id", ident->softid);
    std_setenv("/env/profile/software_id", ident->softid);
    std_setenv("/env/profile/hardware_id", ident->hardid);

    /* Set up the short sidcode string, let dll_entry_init mangle it */

    str_format(
        sidcode_short,
        lengthof(sidcode_short),
        "%s%s%s%s%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev,
        ident->ext);

    /* Set up long-form sidcode env var */

    str_format(
        sidcode_long,
        lengthof(sidcode_long),
        "%s:%s:%s:%s:%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev,
        ident->ext);

    /* Set this up beforehand, as certain games require it in dll_entry_init */

    std_setenv("/env/profile/soft_id_code", sidcode_long);

    ok = module_context_invoke_init(module, sidcode_short, app_config);

    if (!ok) {
        return false;
    }

    /* Back-propagate sidcode, as some games modify it during init */

    memcpy(ident->model, sidcode_short + 0, sizeof(ident->model) - 1);
    ident->dest[0] = sidcode_short[3];
    ident->spec[0] = sidcode_short[4];
    ident->rev[0] = sidcode_short[5];
    memcpy(ident->ext, sidcode_short + 6, sizeof(ident->ext));

    /* Set up long-form sidcode env var again */

    str_format(
        sidcode_long,
        lengthof(sidcode_long),
        "%s:%s:%s:%s:%s",
        ident->model,
        ident->dest,
        ident->spec,
        ident->rev,
        ident->ext);

    std_setenv("/env/profile/soft_id_code", sidcode_long);

    return true;
}

void ea3_ident_to_property(
    const struct ea3_ident *ident, struct property *ea3_config)
{
    struct property_node *node;
    int i;

    for (i = 0; ea3_ident_psmap[i].type != 0xFF; i++) {
        node = property_search(ea3_config, 0, ea3_ident_psmap[i].path);

        if (node != NULL) {
            property_node_remove(node);
        }
    }

    property_psmap_export(ea3_config, NULL, ident, ea3_ident_psmap);
}

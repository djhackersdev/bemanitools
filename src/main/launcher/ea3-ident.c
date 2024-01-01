#include <string.h>

#include "imports/avs.h"

#include "launcher/ea3-ident.h"
#include "launcher/module.h"
#include "launcher/property.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/str.h"

PSMAP_BEGIN(ea3_ident_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct ea3_ident, softid, "/id/softid", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct ea3_ident, hardid, "/id/hardid", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct ea3_ident, pcbid, "/id/pcbid", "")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, model, "/soft/model")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, dest, "/soft/dest")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, spec, "/soft/spec")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, rev, "/soft/rev")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident, ext, "/soft/ext")
PSMAP_END

void ea3_ident_init(struct ea3_ident *ident)
{
    memset(ident, 0, sizeof(*ident));
}

void ea3_ident_initialize_from_file(
        const char *path,
        struct ea3_ident *ea3_ident)
{
    struct property *property;
    struct property_node *node;

    log_assert(path);
    log_assert(ea3_ident);

    property = boot_property_load(path);
    node = property_search(property, NULL, "/ea3_conf");

    if (node == NULL) {
        log_fatal("%s: /ea3_conf missing", path);
    }

    if (!property_psmap_import(property, node, ea3_ident, ea3_ident_psmap)) {
        log_fatal(
            "%s: Error reading IDs from config file", path);
    }

    boot_property_free(property);
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
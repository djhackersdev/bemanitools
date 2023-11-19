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

    if (!property_psmap_import(property, NULL, ea3_ident, ea3_ident_psmap)) {
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

void ea3_ident_replace_property_bool(
    struct property_node *node, const char *name, uint8_t val)
{
    struct property_node *tmp;

    tmp = property_search(NULL, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    property_node_create(NULL, node, PROPERTY_TYPE_BOOL, name, val);
}

void ea3_ident_replace_property_str(
    struct property_node *node, const char *name, const char *val)
{
    struct property_node *tmp;

    tmp = property_search(NULL, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    tmp = property_node_create(NULL, node, PROPERTY_TYPE_STR, name, val);
}

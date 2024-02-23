#define LOG_MODULE "ea3-ident-config"

#include <string.h>

#include "core/log.h"

#include "imports/avs.h"

#include "launcher/ea3-ident-config.h"
#include "launcher/property-util.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/str.h"

#define ROOT_NODE "/ea3_conf"

PSMAP_BEGIN(ea3_ident_config_psmap)
PSMAP_OPTIONAL(
    PSMAP_TYPE_STR, struct ea3_ident_config, softid, "/id/softid", "")
PSMAP_OPTIONAL(
    PSMAP_TYPE_STR, struct ea3_ident_config, hardid, "/id/hardid", "")
PSMAP_OPTIONAL(PSMAP_TYPE_STR, struct ea3_ident_config, pcbid, "/id/pcbid", "")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident_config, model, "/soft/model")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident_config, dest, "/soft/dest")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident_config, spec, "/soft/spec")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident_config, rev, "/soft/rev")
PSMAP_REQUIRED(PSMAP_TYPE_STR, struct ea3_ident_config, ext, "/soft/ext")
PSMAP_END

void ea3_ident_config_init(struct ea3_ident_config *config)
{
    memset(config, 0, sizeof(*config));
}

void ea3_ident_config_from_file_load(
    const char *path, struct ea3_ident_config *config)
{
    struct property *property;

    log_assert(path);
    log_assert(config);

    log_info("Loading from file path: %s", path);

    property = property_util_load(path);

    ea3_ident_config_load(property, config);

    property_util_free(property);
}

void ea3_ident_config_load(
    struct property *property, struct ea3_ident_config *config)
{
    struct property_node *node;

    log_assert(property);
    log_assert(config);

    node = property_search(property, NULL, ROOT_NODE);

    if (node == NULL) {
        log_fatal("Root node '" ROOT_NODE "' missing");
    }

    if (!property_psmap_import(
            property, node, config, ea3_ident_config_psmap)) {
        log_fatal("Error reading config file");
    }
}

bool ea3_ident_config_hardid_is_defined(struct ea3_ident_config *config)
{
    log_assert(config);

    return strlen(config->hardid) > 0;
}

void ea3_ident_config_hardid_from_ethernet_set(struct ea3_ident_config *config)
{
    struct avs_net_interface netif;
    int result;

    log_assert(config);

    result = avs_net_ctrl(1, &netif, sizeof(netif));

    if (result < 0) {
        log_fatal(
            "avs_net_ctrl call to get MAC address returned error: %d", result);
    }

    config->hardid[0] = '0';
    config->hardid[1] = '1';
    config->hardid[2] = '0';
    config->hardid[3] = '0';

    hex_encode_uc(
        netif.mac_addr,
        sizeof(netif.mac_addr),
        config->hardid + 4,
        sizeof(config->hardid) - 4);
}
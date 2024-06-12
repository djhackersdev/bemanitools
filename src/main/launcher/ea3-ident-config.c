#define LOG_MODULE "ea3-ident-config"

#include <string.h>

#include "core/property-node-ext.h"
#include "core/property-node.h"

#include "iface-core/log.h"

#include "imports/avs.h"

#include "launcher/ea3-ident-config.h"

#include "util/defs.h"
#include "util/hex.h"
#include "util/str.h"

#define ROOT_NODE "/ea3_conf"

static void _ea3_ident_config_root_get(
    const core_property_t *property, core_property_node_t *node)
{
    core_property_node_result_t result;
    char node_name[128];

    log_assert(property);
    log_assert(node);

    result = core_property_root_node_get(property, node);
    core_property_fatal_on_error(result);

    result = core_property_node_name_get(node, node_name, sizeof(node_name));
    core_property_node_fatal_on_error(result);

    if (!str_eq(node_name, "ea3_conf")) {
        log_fatal("Root node " ROOT_NODE " in ea3-ident config missing");
    } else {
        core_property_node_fatal_on_error(result);
    }
}

void ea3_ident_config_init(struct ea3_ident_config *config)
{
    memset(config, 0, sizeof(*config));
}

void ea3_ident_config_from_file_load(
    const char *path, struct ea3_ident_config *config)
{
    core_property_t *property;
    core_property_result_t result;

    log_assert(path);
    log_assert(config);

    log_info("Loading from file path: %s", path);

    result = core_property_file_load(path, &property);
    core_property_fatal_on_error(result);

    ea3_ident_config_load(property, config);

    core_property_free(&property);
}

void ea3_ident_config_load(
    const core_property_t *property, struct ea3_ident_config *config)
{
    core_property_node_t node;
    core_property_node_result_t result;

    log_assert(property);
    log_assert(config);

    _ea3_ident_config_root_get(property, &node);

    result = core_property_node_ext_str_read_or_default(
        &node, "id/softid", config->softid, sizeof(config->softid), "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &node, "id/pcbid", config->pcbid, sizeof(config->pcbid), "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &node, "soft/model", config->model, sizeof(config->model), "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &node, "soft/dest", config->dest, sizeof(config->dest), "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &node, "soft/spec", config->spec, sizeof(config->spec), "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &node, "soft/rev", config->rev, sizeof(config->rev), "");
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read_or_default(
        &node, "soft/ext", config->ext, sizeof(config->ext), "");
    core_property_node_fatal_on_error(result);
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
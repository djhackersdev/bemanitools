#define LOG_MODULE "eamuse-config"

#include <stdlib.h>

#include "avs-ext/property-ext.h"

#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "launcher/ea3-ident-config.h"
#include "launcher/eamuse-config.h"

#include "util/str.h"

#define EAMUSE_CONFIG_ROOT_NODE "/ea3"

core_property_t *eamuse_config_avs_load(const char *path)
{
    core_property_t *property;
    core_property_node_t node;

    log_assert(path);

    log_misc("Loading from avs path: %s", path);

    avs_ext_property_ext_avs_file_load(path, &property);

    // Check if root node exists, call already errors if not
    eamuse_config_root_get(property, &node);

    return property;
}

void eamuse_config_root_get(
    core_property_t *property, core_property_node_t *node)
{
    core_property_node_result_t result;
    char node_name[128];

    log_assert(property);
    log_assert(node);

    result = core_property_root_node_get(property, node);
    core_property_node_fatal_on_error(result);

    result = core_property_node_name_get(node, node_name, sizeof(node_name));
    core_property_node_fatal_on_error(result);

    if (!str_eq(node_name, "ea3")) {
        log_fatal("Root node " EAMUSE_CONFIG_ROOT_NODE
                  " in eamuse config missing");
    } else {
        core_property_node_fatal_on_error(result);
    }
}

void eamuse_config_id_softid_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "id/softid", value);
}

void eamuse_config_id_hardid_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "id/hardid", value);
}

void eamuse_config_id_pcbid_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "id/pcbid", value);
}

void eamuse_config_soft_model_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "soft/model", value);
}

void eamuse_config_soft_dest_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "soft/dest", value);
}

void eamuse_config_soft_spec_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "soft/spec", value);
}

void eamuse_config_soft_rev_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "soft/rev", value);
}

void eamuse_config_soft_ext_set(core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "soft/ext", value);
}

void eamuse_config_network_url_slash_set(core_property_node_t *node, bool value)
{
    log_assert(node);

    core_property_node_ext_bool_replace(node, "network/url_slash", value);
}

void eamuse_config_network_service_url_set(
    core_property_node_t *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    core_property_node_ext_str_replace(node, "network/services", value);
}
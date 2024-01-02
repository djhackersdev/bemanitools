#define LOG_MODULE "eamuse-config"

#include <stdlib.h>

#include "imports/avs.h"

#include "launcher/ea3-ident-config.h"
#include "launcher/eamuse-config.h"
#include "launcher/property-util.h"

#include "util/log.h"

#define EAMUSE_CONFIG_ROOT_NODE "/ea3"

struct property* eamuse_config_avs_load(const char *path)
{
    struct property *property;

    log_assert(path);

    log_misc("Loading from avs path: %s", path);

    property = property_util_avs_fs_load(path);

    // Check if root node exists, call already errors if not
    eamuse_config_root_get(property);

    return property;
}

struct property_node* eamuse_config_root_get(struct property *property)
{
    struct property_node *node;

    log_assert(property);

    node = property_search(property, 0, EAMUSE_CONFIG_ROOT_NODE);

    if (node == NULL) {
        log_fatal("Root node " EAMUSE_CONFIG_ROOT_NODE " in eamuse config missing");
    }

    return node;
}

void eamuse_config_id_softid_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "id/softid", value);
}

void eamuse_config_id_hardid_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "id/hardid", value);
}

void eamuse_config_id_pcbid_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "id/pcbid", value);
}

void eamuse_config_soft_model_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "soft/model", value);
}

void eamuse_config_soft_dest_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "soft/dest", value);
}

void eamuse_config_soft_spec_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "soft/spec", value);
}

void eamuse_config_soft_rev_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "soft/rev", value);
}

void eamuse_config_soft_ext_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "soft/ext", value);
}

void eamuse_config_network_url_slash_set(struct property_node *node, bool value)
{
    log_assert(node);

    property_util_node_bool_replace(NULL, node, "network/url_slash", value);
}

void eamuse_config_network_service_url_set(struct property_node *node, const char *value)
{
    log_assert(node);
    log_assert(value);

    property_util_node_str_replace(NULL, node, "network/services", value);
}
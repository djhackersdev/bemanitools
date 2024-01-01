#define LOG_MODULE "eamuse-config"

#include <stdlib.h>

#include "imports/avs.h"

#include "launcher/ea3-ident.h"
#include "launcher/eamuse-config.h"
#include "launcher/property-util.h"

#include "util/log.h"

#define EAMUSE_CONFIG_ROOT_NODE "/ea3"

struct property* eamuse_config_load_from_avs_path(
    const char *avs_path)
{
    struct property *property;

    log_assert(avs_path);

    log_misc("Loading ea3-config from avs path: %s", avs_path);

    property = property_util_load_avs(avs_path);

    // Check if root node exists, call already errors if not
    eamuse_config_resolve_root_node(property);

    return property;
}

struct property_node* eamuse_config_resolve_root_node(struct property *property)
{
    struct property_node *node;

    log_assert(property);

    node = property_search(property, 0, EAMUSE_CONFIG_ROOT_NODE);

    if (node == NULL) {
        log_fatal("Root node " EAMUSE_CONFIG_ROOT_NODE " in eamuse config missing");
    }

    return node;
}

void eamuse_config_inject_ea3_ident(
    struct property *eamuse_property,
    const struct ea3_ident *ea3_ident)
{
    struct property_node *ea3_node;
    struct property_node *node;
    int i;

    log_misc("Injecting ea3_ident data...");

    ea3_node = eamuse_config_resolve_root_node(eamuse_property);

    for (i = 0; ea3_ident_psmap[i].type != PSMAP_TYPE_TERMINATOR; i++) {
        node = property_search(eamuse_property, ea3_node, ea3_ident_psmap[i].path);

        if (node != NULL) {
            property_node_remove(node);
        }
    }

    property_psmap_export(eamuse_property, ea3_node, ea3_ident, ea3_ident_psmap);
}

void eamuse_config_inject_parameters(
    struct property *eamuse_property,
    bool urlslash_enabled,
    bool urlslash_value,
    const char *service_url)
{
    struct property_node *node;

    log_assert(eamuse_property);
    log_assert(service_url);

    node = eamuse_config_resolve_root_node(eamuse_property);

    if (urlslash_enabled) {
        log_misc(
            "Overriding url_slash to: %d", urlslash_value);

        property_util_node_replace_bool(
            eamuse_property,
            node,
            "network/url_slash",
            urlslash_value);
    }

    if (service_url) {
        log_misc("Overriding service url to: %s", service_url);

        property_util_node_replace_str(
            eamuse_property,
            node,
            "network/services",
            service_url);
    }
}
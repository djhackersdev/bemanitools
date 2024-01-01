#ifndef LAUNCHER_EAMUSE_CONFIG_H
#define LAUNCHER_EAMUSE_CONFIG_H

#include "launcher/property.h"

struct property* eamuse_config_load_from_avs_path(const char *avs_path);

struct property_node* eamuse_config_resolve_root_node(struct property *property);

void eamuse_config_inject_ea3_ident(
    struct property *eamuse_property,
    const struct ea3_ident *ea3_ident);

void eamuse_config_inject_parameters(
    struct property *eamuse_property,
    bool urlslash_enabled,
    bool urlslash_value,
    const char *service_url);

#endif
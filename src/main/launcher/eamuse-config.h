#ifndef LAUNCHER_EAMUSE_CONFIG_H
#define LAUNCHER_EAMUSE_CONFIG_H

#include "core/property-node.h"
#include "core/property.h"

core_property_t *eamuse_config_avs_load(const char *path);
void eamuse_config_root_get(
    core_property_t *property, core_property_node_t *node);

void eamuse_config_id_softid_set(core_property_node_t *node, const char *value);
void eamuse_config_id_hardid_set(core_property_node_t *node, const char *value);
void eamuse_config_id_pcbid_set(core_property_node_t *node, const char *value);
void eamuse_config_soft_model_set(
    core_property_node_t *node, const char *value);
void eamuse_config_soft_dest_set(core_property_node_t *node, const char *value);
void eamuse_config_soft_spec_set(core_property_node_t *node, const char *value);
void eamuse_config_soft_rev_set(core_property_node_t *node, const char *value);
void eamuse_config_soft_ext_set(core_property_node_t *node, const char *value);
void eamuse_config_network_url_slash_set(
    core_property_node_t *node, bool value);
void eamuse_config_network_service_url_set(
    core_property_node_t *node, const char *value);

#endif
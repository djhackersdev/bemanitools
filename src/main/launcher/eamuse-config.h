#ifndef LAUNCHER_EAMUSE_CONFIG_H
#define LAUNCHER_EAMUSE_CONFIG_H

#include "imports/avs.h"

struct property *eamuse_config_avs_load(const char *path);
struct property_node *eamuse_config_root_get(struct property *property);

void eamuse_config_id_softid_set(struct property_node *node, const char *value);
void eamuse_config_id_hardid_set(struct property_node *node, const char *value);
void eamuse_config_id_pcbid_set(struct property_node *node, const char *value);
void eamuse_config_soft_model_set(
    struct property_node *node, const char *value);
void eamuse_config_soft_dest_set(struct property_node *node, const char *value);
void eamuse_config_soft_spec_set(struct property_node *node, const char *value);
void eamuse_config_soft_rev_set(struct property_node *node, const char *value);
void eamuse_config_soft_ext_set(struct property_node *node, const char *value);
void eamuse_config_network_url_slash_set(
    struct property_node *node, bool value);
void eamuse_config_network_service_url_set(
    struct property_node *node, const char *value);

#endif
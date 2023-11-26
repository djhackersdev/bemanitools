#ifndef LAUNCHER_PROPERTY_H
#define LAUNCHER_PROPERTY_H

#include "imports/avs.h"

void boot_property_log(struct property *property);
void boot_property_node_log(struct property_node *node);
struct property *boot_property_load(const char *filename);
struct property *boot_property_load_avs(const char *filename);
struct property *boot_property_load_cstring(const char *cstring);
void boot_property_node_replace_u8(struct property *property,
    struct property_node *node, const char *name, uint8_t val);
void boot_property_node_replace_bool(struct property *property,
    struct property_node *node, const char *name, bool val);
void boot_property_node_replace_str(struct property *property,
    struct property_node *node, const char *name, const char *val);
void boot_property_free(struct property *prop);

#endif

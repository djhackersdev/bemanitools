#ifndef PROPERTY_UTIL_H
#define PROPERTY_UTIL_H

#include "imports/avs.h"

void property_util_log(struct property *property);
void property_util_node_log(struct property_node *node);
struct property *property_util_load_file(const char *filename);
struct property *property_util_load_avs(const char *filename);
struct property *property_util_load_cstring(const char *cstring);
void property_util_node_replace_u8(struct property *property,
    struct property_node *node, const char *name, uint8_t val);
void property_util_node_replace_bool(struct property *property,
    struct property_node *node, const char *name, bool val);
void property_util_node_replace_str(struct property *property,
    struct property_node *node, const char *name, const char *val);
void property_util_free(struct property *prop);

#endif

#ifndef PROPERTY_UTIL_H
#define PROPERTY_UTIL_H

#include "imports/avs.h"

void property_util_log(struct property *property);
void property_util_node_log(struct property_node *node);
struct property *property_util_load(const char *filename);
struct property *property_util_avs_fs_load(const char *filename);
struct property *property_util_cstring_load(const char *cstring);
uint32_t property_util_property_query_real_size(struct property *property);
void property_util_node_u8_replace(struct property *property,
    struct property_node *node, const char *name, uint8_t val);
void property_util_node_u16_replace(struct property *property,
    struct property_node *node, const char *name, uint16_t val);
void property_util_node_u32_replace(struct property *property,
    struct property_node *node, const char *name, uint32_t val);
void property_util_node_str_replace(struct property *property,
    struct property_node *node, const char *name, const char *val);
void property_util_node_bool_replace(struct property *property,
    struct property_node *node, const char *name, bool val);
struct property* property_util_clone(struct property_node *node);
struct property* property_util_merge(struct property **properties, size_t count);
void property_util_node_merge(
    struct property *parent_property,
    struct property_node *parent_node,
    struct property_node *source_node);
void property_util_free(struct property *prop);

#endif

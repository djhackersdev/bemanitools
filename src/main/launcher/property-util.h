#ifndef PROPERTY_UTIL_H
#define PROPERTY_UTIL_H

#include <stdbool.h>

#include "imports/avs.h"

// Guestimate, should be long enough, I hope?
#define PROPERTY_NODE_PATH_LEN_MAX 4096
// 256 found in AVS code as size used on property_node_name
#define PROPERTY_NODE_NAME_SIZE_MAX 256
// Guestimate, should be enough, I hope?
#define PROPERTY_NODE_ATTR_NAME_SIZE_MAX 128

#define PROPERTY_UTIL_MAX_NODE_NAME_RESOLVERS 4

typedef void (*property_util_node_merge_recursion_do_t)(
    struct property *parent_property,
    struct property_node *parent,
    struct property_node *source,
    void *ctx);

typedef bool (*property_util_node_merge_strategy_do_t)(
    struct property *parent_property,
    struct property_node *parent,
    struct property_node *source,
    void *ctx,
    property_util_node_merge_recursion_do_t node_merge_recursion_do);

struct property_util_node_merge_strategies {
    struct {
        const char *path;
        property_util_node_merge_strategy_do_t merge_strategy_do;
    } entry[PROPERTY_UTIL_MAX_NODE_NAME_RESOLVERS];
    uint8_t num;
};

void property_util_log(struct property *property);
void property_util_node_log(struct property_node *node);
struct property *property_util_load(const char *filename);
struct property *property_util_avs_fs_load(const char *filename);
struct property *property_util_cstring_load(const char *cstring);
struct property *property_util_clone(struct property *property);
void property_util_free(struct property *prop);
uint32_t property_util_property_query_real_size(struct property *property);
void property_util_node_u8_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    uint8_t val);
void property_util_node_u16_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    uint16_t val);
void property_util_node_u32_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    uint32_t val);
void property_util_node_str_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    const char *val);
void property_util_node_bool_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    bool val);
void property_util_node_attribute_replace(
    struct property *property,
    struct property_node *node,
    const char *name,
    const char *val);

struct property *
property_util_many_merge(struct property **properties, size_t count);
struct property *property_util_node_extract(struct property_node *node);

struct property *
property_util_merge(struct property *parent, struct property *source);

// Strategies are applied in order and first consumer terminates
// applying further strategies Typically, you want to include the default
// strategy after your custom strategies for special cases
struct property *property_util_merge_with_strategies(
    struct property *parent,
    struct property *source,
    const struct property_util_node_merge_strategies *strategies);

bool property_util_node_merge_default_strategy_do(
    struct property *parent_property,
    struct property_node *parent,
    struct property_node *source,
    void *ctx,
    property_util_node_merge_recursion_do_t node_merge_recursion_do);

#endif

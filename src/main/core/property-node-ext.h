#ifndef CORE_PROPERTY_NODE_EXT_H
#define CORE_PROPERTY_NODE_EXT_H

#include <stdbool.h>
#include <stdint.h>

#include "core/property-node.h"

#define CORE_PROPERTY_NODE_EXT_MAX_NODE_NAME_RESOLVERS 4

typedef core_property_node_result_t (*core_property_node_ext_merge_recursion_do_t)(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx,
    bool *consumed);

typedef core_property_node_result_t (*core_property_node_ext_merge_strategy_do_t)(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx,
    bool *consumed,
    core_property_node_ext_merge_recursion_do_t node_merge_recursion_do);

struct core_property_node_ext_merge_strategies {
    struct {
        const char *path;
        core_property_node_ext_merge_strategy_do_t merge_strategy_do;
    } entry[CORE_PROPERTY_NODE_EXT_MAX_NODE_NAME_RESOLVERS];
    uint8_t num;
};

typedef struct core_property_node_ext_merge_strategies core_property_node_ext_merge_strategies_t;

core_property_node_result_t core_property_node_ext_u8_replace(
    core_property_node_t *node,
    const char *name,
    uint8_t val);
core_property_node_result_t core_property_node_ext_u16_replace(
    core_property_node_t *node,
    const char *name,
    uint16_t val);
core_property_node_result_t core_property_node_ext_u32_replace(
    core_property_node_t *node,
    const char *name,
    uint32_t val);
core_property_node_result_t core_property_node_ext_str_replace(
    core_property_node_t *node,
    const char *name,
    const char *val);
core_property_node_result_t core_property_node_ext_bool_replace(
    core_property_node_t *node,
    const char *name,
    bool val);
core_property_node_result_t core_property_node_ext_attr_replace(
    core_property_node_t *node,
    const char *name,
    const char *val);

core_property_node_result_t core_property_node_ext_extract(
        const core_property_node_t *node, core_property_t **out_property);

// Strategies are applied in order and first consumer terminates
// applying further strategies Typically, you want to include the default
// strategy after your custom strategies for special cases
core_property_node_result_t core_property_node_ext_merge_do(
    const core_property_t *parent,
    const core_property_t *source,
    const core_property_node_ext_merge_strategies_t *strategies,
    core_property_t **merged);

core_property_node_result_t core_property_node_ext_merge_default_strategy_do(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx,
    bool *consumed,
    core_property_node_ext_merge_recursion_do_t node_merge_recursion_do);

#endif
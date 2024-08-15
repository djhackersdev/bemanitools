#ifndef CORE_PROPERTY_NODE_EXT_H
#define CORE_PROPERTY_NODE_EXT_H

#include <stdbool.h>
#include <stdint.h>

#include "core/property-node.h"

#define CORE_PROPERTY_NODE_EXT_MAX_NODE_NAME_RESOLVERS 4

typedef core_property_node_result_t (
    *core_property_node_ext_merge_recursion_do_t)(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx);

typedef core_property_node_result_t (
    *core_property_node_ext_merge_strategy_do_t)(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx,
    bool *consumed,
    core_property_node_ext_merge_recursion_do_t node_merge_recursion_do);

typedef struct core_property_node_ext_merge_strategies {
    struct {
        const char *path;
        core_property_node_ext_merge_strategy_do_t merge_strategy_do;
    } entry[CORE_PROPERTY_NODE_EXT_MAX_NODE_NAME_RESOLVERS];
    uint8_t num;
} core_property_node_ext_merge_strategies_t;

void core_property_node_ext_log(
    const core_property_node_t *node, bt_core_log_message_t log_message);

core_property_node_result_t core_property_node_ext_u8_read(
    const core_property_node_t *node, const char *name, uint8_t *value);
core_property_node_result_t core_property_node_ext_u16_read(
    const core_property_node_t *node, const char *name, uint16_t *value);
core_property_node_result_t core_property_node_ext_u32_read(
    const core_property_node_t *node, const char *name, uint32_t *value);
core_property_node_result_t core_property_node_ext_bool_read(
    const core_property_node_t *node, const char *name, bool *value);
core_property_node_result_t core_property_node_ext_str_read(
    const core_property_node_t *node,
    const char *name,
    char *value,
    size_t len);

core_property_node_result_t core_property_node_ext_u8_read_or_default(
    const core_property_node_t *node,
    const char *name,
    uint8_t *value,
    uint8_t _default);
core_property_node_result_t core_property_node_ext_u16_read_or_default(
    const core_property_node_t *node,
    const char *name,
    uint16_t *value,
    uint16_t _default);
core_property_node_result_t core_property_node_ext_u32_read_or_default(
    const core_property_node_t *node,
    const char *name,
    uint32_t *value,
    uint32_t _default);
core_property_node_result_t core_property_node_ext_bool_read_or_default(
    const core_property_node_t *node,
    const char *name,
    bool *value,
    bool _default);
core_property_node_result_t core_property_node_ext_str_read_or_default(
    const core_property_node_t *node,
    const char *name,
    char *value,
    size_t len,
    const char *_default);

core_property_node_result_t core_property_node_ext_u8_replace(
    core_property_node_t *node, const char *name, uint8_t val);
core_property_node_result_t core_property_node_ext_u16_replace(
    core_property_node_t *node, const char *name, uint16_t val);
core_property_node_result_t core_property_node_ext_u32_replace(
    core_property_node_t *node, const char *name, uint32_t val);
core_property_node_result_t core_property_node_ext_str_replace(
    core_property_node_t *node, const char *name, const char *val);
core_property_node_result_t core_property_node_ext_bool_replace(
    core_property_node_t *node, const char *name, bool val);
core_property_node_result_t core_property_node_ext_attr_replace(
    core_property_node_t *node, const char *name, const char *val);

core_property_node_result_t core_property_node_ext_extract(
    const core_property_node_t *node, core_property_t **out_property);

core_property_node_result_t core_property_node_ext_merge_do(
    const core_property_t *parent,
    const core_property_t *source,
    core_property_t **out_property);
// Strategies are applied in order and first consumer terminates
// applying further strategies Typically, you want to include the default
// strategy after your custom strategies for special cases
core_property_node_result_t core_property_node_ext_merge_with_strategies_do(
    const core_property_t *parent,
    const core_property_t *source,
    const core_property_node_ext_merge_strategies_t *strategies,
    core_property_t **merged);

core_property_node_result_t core_property_node_ext_merge_strategy_default_do(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx,
    bool *consumed,
    core_property_node_ext_merge_recursion_do_t node_merge_recursion_do);

#endif
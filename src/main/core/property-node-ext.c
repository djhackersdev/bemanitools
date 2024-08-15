#define LOG_MODULE "core-property-node-ext"

#include "iface-core/log.h"

#include "core/property-node-ext.h"
#include "core/property-node.h"

#include "util/str.h"

struct core_property_node_ext_merge_ctx {
    const char *path;
    const core_property_node_ext_merge_strategies_t *strategies;
};

typedef struct core_property_node_ext_merge_ctx
    core_property_node_ext_merge_ctx_t;

static core_property_node_result_t _core_property_node_ext_merge_recursive_do(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx_)
{
    const core_property_node_ext_merge_ctx_t *ctx;
    core_property_node_ext_merge_ctx_t ctx_next;

    char parent_name[CORE_PROPERTY_NODE_NAME_SIZE_MAX];
    char parent_path[CORE_PROPERTY_NODE_PATH_LEN_MAX];

    core_property_node_result_t result;
    bool consumed;
    uint8_t i;

    log_assert(parent);
    log_assert(source);
    log_assert(ctx_);

    ctx = (const core_property_node_ext_merge_ctx_t *) ctx_;

    log_assert(ctx->path);
    log_assert(ctx->strategies);
    log_assert(ctx->strategies->num > 0);

    result =
        core_property_node_name_get(parent, parent_name, sizeof(parent_name));

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    str_cpy(parent_path, sizeof(parent_path), ctx->path);
    str_cat(parent_path, sizeof(parent_path), "/");
    str_cat(parent_path, sizeof(parent_path), parent_name);

    ctx_next.path = parent_path;
    ctx_next.strategies = ctx->strategies;

    consumed = false;

    // Apply given strategies, one MUST consume
    for (i = 0; i < ctx->strategies->num; i++) {
        log_assert(ctx->strategies->entry[i].path);

        // path == "" matches everything
        if (str_eq(ctx->strategies->entry[i].path, "") ||
            str_eq(ctx->strategies->entry[i].path, parent_path)) {

            result = ctx->strategies->entry[i].merge_strategy_do(
                parent,
                source,
                &ctx_next,
                &consumed,
                _core_property_node_ext_merge_recursive_do);

            if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                return result;
            }

            log_misc(
                "Merge strategy for '%s' consumed: %d",
                ctx->strategies->entry[i].path,
                consumed);

            if (consumed) {
                break;
            }
        }
    }

    log_assert(consumed);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

core_property_node_result_t core_property_node_ext_u8_read(
    const core_property_node_t *node, const char *name, uint8_t *value)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_search(node, name, &tmp);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    return core_property_node_u8_read(&tmp, value);
}

core_property_node_result_t core_property_node_ext_u16_read(
    const core_property_node_t *node, const char *name, uint16_t *value)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_search(node, name, &tmp);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    return core_property_node_u16_read(&tmp, value);
}

core_property_node_result_t core_property_node_ext_u32_read(
    const core_property_node_t *node, const char *name, uint32_t *value)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_search(node, name, &tmp);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    return core_property_node_u32_read(&tmp, value);
}

core_property_node_result_t core_property_node_ext_bool_read(
    const core_property_node_t *node, const char *name, bool *value)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_search(node, name, &tmp);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    return core_property_node_bool_read(&tmp, value);
}

core_property_node_result_t core_property_node_ext_str_read(
    const core_property_node_t *node, const char *name, char *value, size_t len)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);
    log_assert(len > 0);

    result = core_property_node_search(node, name, &tmp);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    return core_property_node_str_read(&tmp, value, len);
}

core_property_node_result_t core_property_node_ext_attr_read(
    const core_property_node_t *node, const char *name, char *value, size_t len)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);
    log_assert(len > 0);

    result = core_property_node_search(node, name, &tmp);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    return core_property_node_attr_read(&tmp, value, len);
}

core_property_node_result_t core_property_node_ext_u8_read_or_default(
    const core_property_node_t *node,
    const char *name,
    uint8_t *value,
    uint8_t _default)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_ext_u8_read(node, name, value);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        *value = _default;

        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    } else {
        return result;
    }
}

core_property_node_result_t core_property_node_ext_u16_read_or_default(
    const core_property_node_t *node,
    const char *name,
    uint16_t *value,
    uint16_t _default)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_ext_u16_read(node, name, value);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        *value = _default;

        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    } else {
        return result;
    }
}

core_property_node_result_t core_property_node_ext_u32_read_or_default(
    const core_property_node_t *node,
    const char *name,
    uint32_t *value,
    uint32_t _default)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_ext_u32_read(node, name, value);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        *value = _default;

        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    } else {
        return result;
    }
}

core_property_node_result_t core_property_node_ext_bool_read_or_default(
    const core_property_node_t *node,
    const char *name,
    bool *value,
    bool _default)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);

    result = core_property_node_ext_bool_read(node, name, value);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        *value = _default;

        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    } else {
        return result;
    }
}

core_property_node_result_t core_property_node_ext_str_read_or_default(
    const core_property_node_t *node,
    const char *name,
    char *value,
    size_t len,
    const char *_default)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);
    log_assert(value);
    log_assert(len > 0);
    log_assert(_default);

    result = core_property_node_ext_str_read(node, name, value, len);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        str_cpy(value, len, _default);

        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    } else {
        return result;
    }
}

core_property_node_result_t core_property_node_ext_u8_replace(
    core_property_node_t *node, const char *name, uint8_t val)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);

    result = core_property_node_search(node, name, &tmp);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }

        result = core_property_node_remove(&tmp);

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }
    }

    return core_property_node_u8_create(node, name, val, NULL);
}

core_property_node_result_t core_property_node_ext_u16_replace(
    core_property_node_t *node, const char *name, uint16_t val)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);

    result = core_property_node_search(node, name, &tmp);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }

        result = core_property_node_remove(&tmp);

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }
    }

    return core_property_node_u16_create(node, name, val, NULL);
}

core_property_node_result_t core_property_node_ext_u32_replace(
    core_property_node_t *node, const char *name, uint32_t val)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);

    result = core_property_node_search(node, name, &tmp);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }

        result = core_property_node_remove(&tmp);

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }
    }

    return core_property_node_u32_create(node, name, val, NULL);
}

core_property_node_result_t core_property_node_ext_str_replace(
    core_property_node_t *node, const char *name, const char *val)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);

    result = core_property_node_search(node, name, &tmp);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }

        result = core_property_node_remove(&tmp);

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }
    }

    return core_property_node_str_create(node, name, val, NULL);
}

core_property_node_result_t core_property_node_ext_bool_replace(
    core_property_node_t *node, const char *name, bool val)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);

    result = core_property_node_search(node, name, &tmp);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }

        result = core_property_node_remove(&tmp);

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }
    }

    return core_property_node_bool_create(node, name, val, NULL);
}

core_property_node_result_t core_property_node_ext_attr_replace(
    core_property_node_t *node, const char *name, const char *val)
{
    core_property_node_t tmp;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(name);

    result = core_property_node_search(node, name, &tmp);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }

        result = core_property_node_remove(&tmp);

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            return result;
        }
    }

    return core_property_node_attr_create(node, name, val, NULL);
}

core_property_node_result_t core_property_node_ext_extract(
    const core_property_node_t *node, core_property_t **out_property)
{
    size_t size;
    core_property_node_result_t result;
    core_property_t *property;
    core_property_result_t property_result;

    log_assert(node);
    log_assert(out_property);

    result = core_property_node_size(node, &size);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        return result;
    }

    property_result = core_property_create(size, &property);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    result = core_property_other_node_insert(property, node);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        core_property_free(&property);

        return result;
    }

    *out_property = property;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

core_property_node_result_t core_property_node_ext_merge_do(
    const core_property_t *parent,
    const core_property_t *source,
    core_property_t **out_property)
{
    core_property_node_ext_merge_strategies_t strategies;

    log_assert(parent);
    log_assert(source);
    log_assert(out_property);

    strategies.num = 1;

    strategies.entry[0].path = "";
    strategies.entry[0].merge_strategy_do =
        core_property_node_ext_merge_strategy_default_do;

    return core_property_node_ext_merge_with_strategies_do(
        parent, source, &strategies, out_property);
}

core_property_node_result_t core_property_node_ext_merge_with_strategies_do(
    const core_property_t *parent,
    const core_property_t *source,
    const core_property_node_ext_merge_strategies_t *strategies,
    core_property_t **merged)
{
    size_t size;
    size_t total_size;
    core_property_result_t property_result;
    core_property_node_result_t property_node_result;
    core_property_t *merged_property;
    core_property_node_t parent_node;
    core_property_node_t merged_node;
    core_property_node_t source_node;
    core_property_node_ext_merge_ctx_t ctx;

    log_assert(parent);
    log_assert(source);
    log_assert(strategies);

    // We can't estimate how these two are being merged as in how much new
    // data is being inserted from source into parent. Therefore, worse-case
    // estimate memory requirement for no overlap
    total_size = 0;

    property_result = core_property_size(parent, &size);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    total_size += size;

    property_result = core_property_size(source, &size);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    total_size += size;

    property_result = core_property_create(total_size, &merged_property);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    property_result = core_property_root_node_get(parent, &parent_node);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        core_property_free(&merged_property);

        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    property_result =
        core_property_other_node_insert(merged_property, &parent_node);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        core_property_free(&merged_property);

        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    property_result =
        core_property_root_node_get(merged_property, &merged_node);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        core_property_free(&merged_property);

        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    property_result = core_property_root_node_get(source, &source_node);

    if (CORE_PROPERTY_RESULT_IS_ERROR(property_result)) {
        core_property_free(&merged_property);

        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    core_property_node_log(&merged_node, log_misc_func);
    core_property_node_log(&source_node, log_misc_func);

    ctx.path = "";
    ctx.strategies = strategies;

    property_node_result = _core_property_node_ext_merge_recursive_do(
        &merged_node, &source_node, &ctx);

    core_property_node_log(&merged_node, log_misc_func);

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(property_node_result)) {
        core_property_free(&merged_property);
    }

    *merged = merged_property;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

core_property_node_result_t core_property_node_ext_merge_strategy_default_do(
    core_property_node_t *parent,
    const core_property_node_t *source,
    void *ctx,
    bool *consumed,
    core_property_node_ext_merge_recursion_do_t node_merge_recursion_do)
{
    core_property_node_t tmp;
    core_property_node_t source_child;
    core_property_node_result_t result;
    char source_child_name[CORE_PROPERTY_NODE_NAME_SIZE_MAX];
    core_property_node_t parent_child;
    core_property_node_t source_child_child;

    log_assert(parent);
    log_assert(source);
    log_assert(consumed);

    result = core_property_node_child_get(source, &source_child);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        *consumed = true;
        return CORE_PROPERTY_NODE_RESULT_SUCCESS;
    }

    if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        *consumed = false;
        return result;
    }

    while (true) {
        result = core_property_node_name_get(
            &source_child, source_child_name, sizeof(source_child_name));

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            *consumed = false;
            return result;
        }

        result =
            core_property_node_search(parent, source_child_name, &parent_child);

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            // Could not find an identical child on parent, copy entire source
            // to parent
            result = core_property_node_copy(parent, &source_child);

            if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                *consumed = false;
                return result;
            }
        } else if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            *consumed = false;
            return result;
        } else {
            // Go deeper, more levels to traverse
            result = core_property_node_child_get(
                &source_child, &source_child_child);

            if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                // Found identical leaf node, remove the matching parent's child
                // and copy the source child over to the parent and terminate
                // the recursion
                result = core_property_node_remove(&parent_child);

                if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                    *consumed = false;
                    return result;
                }

                result = core_property_node_copy(parent, &source_child);

                if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                    *consumed = false;
                    return result;
                }
            } else if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                *consumed = false;
                return result;
            } else {
                // Continue recursion if there are actually more children
                result =
                    node_merge_recursion_do(&parent_child, &source_child, ctx);

                if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
                    *consumed = false;
                    return result;
                }
            }
        }

        // Iterate siblings on same level
        result = core_property_node_next_sibling_get(&source_child, &tmp);
        memcpy(&source_child, &tmp, sizeof(core_property_node_t));

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            break;
        }

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            *consumed = false;
            return result;
        }
    }

    // Default strategy always consumes when successful
    *consumed = true;
    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

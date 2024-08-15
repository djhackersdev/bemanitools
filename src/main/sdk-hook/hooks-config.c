#define LOG_MODULE "bt-hook-inject-hooks-config"

#include "core/property-ext.h"
#include "core/property-node-ext.h"

#include "iface-core/log.h"

#include "sdk-hook/hooks-config.h"

#define BT_HOOK_HOOKS_CONFIG_MAX_LAYER_CONFIG_NODES 8

static core_property_t *
_bt_hook_hooks_config_layered_config_nodes_load(const core_property_node_t *node)
{
    char kind[64];
    char file[MAX_PATH];
    int cnt;

    core_property_node_t cur;
    core_property_node_t tmp;
    core_property_t *config_property[BT_HOOK_HOOKS_CONFIG_MAX_LAYER_CONFIG_NODES];
    core_property_t *merged_property;
    core_property_node_result_t result;
    core_property_result_t prop_result;

    log_assert(node);

    cnt = 0;
    result = core_property_node_search(node, "config", &cur);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        core_property_node_fatal_on_error(result);
    }

    while (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (cnt >= BT_HOOK_HOOKS_CONFIG_MAX_LAYER_CONFIG_NODES) {
            log_fatal(
                "Exceeding max supported config nodes for layering, max is %d",
                BT_HOOK_HOOKS_CONFIG_MAX_LAYER_CONFIG_NODES);
        }

        result =
            core_property_node_attr_read(&cur, "kind", kind, sizeof(kind));

        if (CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
            log_fatal("Failed reading 'kind' attribute value of config node");
        }

        if (!strcmp(kind, "file")) {
            core_property_node_str_read(&cur, file, sizeof(file));

            prop_result = core_property_file_load(file, &config_property[cnt]);
            core_property_fatal_on_error(prop_result);
        } else if (!strcmp(kind, "inline")) {
            // The nested child is the actual root of the inline, not the outer
            // <config> node
            result = core_property_node_child_get(&cur, &tmp);
            memcpy(&cur, &tmp, sizeof(core_property_node_t));

            if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
                core_property_node_fatal_on_error(result);
            }

            result =
                core_property_node_ext_extract(&cur, &config_property[cnt]);
            core_property_node_fatal_on_error(result);
        } else {
            log_fatal(
                "Unsupported 'kind' attribute value '%s' of config node", kind);
        }

        cnt++;
        result = core_property_node_next_result_search(&cur, &tmp);
        memcpy(&cur, &tmp, sizeof(core_property_node_t));

        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    }

    if (cnt == 0) {
        prop_result = core_property_str_load("<hook></hook>", &merged_property);
        core_property_fatal_on_error(prop_result);
    } else {
        prop_result =
            core_property_ext_many_merge(config_property, cnt, &merged_property);
        core_property_fatal_on_error(prop_result);

        for (int i = 0; i < cnt; i++) {
            core_property_free(&config_property[i]);
        }
    }

    return merged_property;
}

static void _bt_hook_hooks_config_hooks_load(
        const core_property_node_t *node,
        struct bt_hook_hooks_hook_config *configs)
{
    core_property_node_result_t result;
    core_property_node_t child;
    core_property_node_t tmp;
    uint8_t processed_hooks;

    processed_hooks = 0;

    result = core_property_node_search(node, "hook", &child);

    do {
        if (processed_hooks >= BT_HOOK_HOOKS_CONFIG_MAX_HOOKS) {
            log_fatal("Cannot load more hooks, max supported capacity reached");
        }

        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            return;
        } else {
            core_property_node_fatal_on_error(result);
        }

        result = core_property_node_ext_bool_read(&child, "enable", &configs[processed_hooks].enable);
        core_property_node_fatal_on_error(result);

        result = core_property_node_ext_str_read(&child, "path", configs[processed_hooks].path, sizeof(configs[processed_hooks].path));
        core_property_node_fatal_on_error(result);

        configs[processed_hooks].config = _bt_hook_hooks_config_layered_config_nodes_load(&child);

        result = core_property_node_next_result_search(&child, &tmp);
        memcpy(&child, &tmp, sizeof(core_property_node_t));

        processed_hooks++;
    } while (true);
}

void bt_hook_hooks_config_init(bt_hook_hooks_config_t *config)
{
    log_assert(config);

    memset(config, 0, sizeof(bt_hook_hooks_config_t));
}

void bt_hook_hooks_config_load(
    const core_property_node_t *node, bt_hook_hooks_config_t *config)
{
    log_assert(node);
    log_assert(config);

    _bt_hook_hooks_config_hooks_load(node, config->hooks);
}

void bt_hook_hooks_config_fini(bt_hook_hooks_config_t *config)
{
    uint32_t i;

    log_assert(config);

    for (i = 0; i < BT_HOOK_HOOKS_CONFIG_MAX_HOOKS; i++) {
        core_property_free(&config->hooks[i].config);
    }
}
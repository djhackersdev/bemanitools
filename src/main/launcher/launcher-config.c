#define LOG_MODULE "launcher-config"

#include <string.h>

#include "core/property-ext.h"
#include "core/property-node-ext.h"
#include "core/property-node.h"
#include "core/property.h"

#include "iface-core/log.h"

#include "launcher/launcher-config.h"

#include "util/mem.h"
#include "util/str.h"

#define ROOT_NODE "/launcher"
#define MAX_LAYER_CONFIG_NODES 8

#define NODE_MISSING_FATAL(subnode) \
    log_fatal("%s/%s: Node missing", ROOT_NODE, subnode);
#define NODE_LOADING_FATAL(subnode) \
    log_fatal("%s/%s: Node loading", ROOT_NODE, subnode);

static core_property_t *
_launcher_config_layered_config_nodes_load(const core_property_node_t *node)
{
    char kind[64];
    char file[MAX_PATH];
    int cnt;

    core_property_node_t cur;
    core_property_node_t tmp;
    core_property_t *config_property[MAX_LAYER_CONFIG_NODES];
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
        if (cnt >= MAX_LAYER_CONFIG_NODES) {
            log_fatal(
                "Exceeding max supported config nodes for layering, max is %d",
                MAX_LAYER_CONFIG_NODES);
        }

        result =
            core_property_node_ext_attr_read(&cur, "kind@", kind, sizeof(kind));

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
        return NULL;
    }

    prop_result =
        core_property_ext_many_merge(config_property, cnt, &merged_property);
    core_property_fatal_on_error(prop_result);

    for (int i = 0; i < cnt; i++) {
        core_property_free(&config_property[i]);
    }

    return merged_property;
}

static void _launcher_config_bootstrap_load(
    const core_property_node_t *node, struct launcher_bootstrap_config *config)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(config);

    result = core_property_node_ext_str_read(
        node, "selector", config->selector, sizeof(config->selector));

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        NODE_MISSING_FATAL("bootstrap/selector");
    } else {
        core_property_node_fatal_on_error(result);
    }

    config->property = _launcher_config_layered_config_nodes_load(node);

    if (config->property == NULL) {
        NODE_MISSING_FATAL("bootstrap/config");
    }
}

static void _launcher_config_hook_hook_load(
    const core_property_node_t *node, struct launcher_hook_config_hook *config)
{
    core_property_node_result_t result;
    core_property_result_t prop_result;

    result = core_property_node_ext_bool_read(node, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(
        node, "path", config->path, sizeof(config->path));
    core_property_node_fatal_on_error(result);

    config->property = _launcher_config_layered_config_nodes_load(node);

    // Default empty config
    if (!config->property) {
        prop_result = core_property_str_load(
            "<hook><version __type\"u32\">1</version></hook>",
            &config->property);
        core_property_fatal_on_error(prop_result);
    }
}

static void _launcher_config_hook_pre_avs_load(
    const core_property_node_t *node,
    struct launcher_hook_config_pre_avs *config)
{
    core_property_node_result_t result;
    core_property_result_t prop_result;

    result = core_property_node_ext_bool_read(node, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(
        node, "path", config->path, sizeof(config->path));
    core_property_node_fatal_on_error(result);

    config->property = _launcher_config_layered_config_nodes_load(node);

    // Default empty config
    if (!config->property) {
        prop_result = core_property_str_load(
            "<hook><version __type\"u32\">1</version></hook>",
            &config->property);
        core_property_fatal_on_error(prop_result);
    }
}

static void _launcher_config_hook_iat_load(
    const core_property_node_t *node, struct launcher_hook_config_iat *config)
{
    core_property_node_result_t result;
    core_property_result_t prop_result;

    result = core_property_node_ext_bool_read(node, "enable", &config->enable);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(
        node, "source_name", config->source_name, sizeof(config->source_name));
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_str_read(
        node, "path", config->path, sizeof(config->path));
    core_property_node_fatal_on_error(result);

    config->property = _launcher_config_layered_config_nodes_load(node);

    // Default empty config
    if (!config->property) {
        prop_result = core_property_str_load(
            "<hook><version __type\"u32\">1</version></hook>",
            &config->property);
        core_property_fatal_on_error(prop_result);
    }
}

static void _launcher_config_hooks_hook_load(
    const core_property_node_t *node, struct launcher_hook_config *config)
{
    int cnt;
    core_property_node_t tmp;
    core_property_node_t cur;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(config);

    cnt = 0;
    result = core_property_node_search(node, "hook", &cur);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        core_property_node_fatal_on_error(result);
    }

    while (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (cnt >= LAUNCHER_CONFIG_MAX_HOOKS) {
            log_warning(
                "Currently not supporting more than %d hooks, skipping "
                "remaining",
                cnt);
            break;
        }

        _launcher_config_hook_hook_load(&cur, &config->hook[cnt]);

        cnt++;
        result = core_property_node_next_result_search(&cur, &tmp);
        memcpy(&cur, &tmp, sizeof(core_property_node_t));

        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    }
}

static void _launcher_config_hooks_pre_avs_load(
    const core_property_node_t *node, struct launcher_hook_config *config)
{
    int cnt;
    core_property_node_t tmp;
    core_property_node_t cur;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(config);

    cnt = 0;
    result = core_property_node_search(node, "pre_avs", &cur);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        core_property_node_fatal_on_error(result);
    }

    while (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (cnt >= LAUNCHER_CONFIG_MAX_HOOKS) {
            log_warning(
                "Currently not supporting more than %d hooks, skipping "
                "remaining",
                cnt);
            break;
        }

        _launcher_config_hook_pre_avs_load(&cur, &config->pre_avs[cnt]);

        cnt++;
        result = core_property_node_next_result_search(&cur, &tmp);
        memcpy(&cur, &tmp, sizeof(core_property_node_t));

        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    }
}

static void _launcher_config_hooks_iat_load(
    const core_property_node_t *node, struct launcher_hook_config *config)
{
    int cnt;
    core_property_node_t tmp;
    core_property_node_t cur;
    core_property_node_result_t result;

    log_assert(node);
    log_assert(config);

    cnt = 0;
    result = core_property_node_search(node, "iat", &cur);

    if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        core_property_node_fatal_on_error(result);
    }

    while (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        if (cnt >= LAUNCHER_CONFIG_MAX_HOOKS) {
            log_warning(
                "Currently not supporting more than %d hooks, skipping "
                "remaining",
                cnt);
            break;
        }

        _launcher_config_hook_iat_load(&cur, &config->iat[cnt]);

        cnt++;
        result = core_property_node_next_result_search(&cur, &tmp);
        memcpy(&cur, &tmp, sizeof(core_property_node_t));

        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    }
}

static void _launcher_config_hooks_load(
    const core_property_node_t *node, struct launcher_hook_config *config)
{
    log_assert(node);
    log_assert(config);

    _launcher_config_hooks_hook_load(node, config);
    _launcher_config_hooks_pre_avs_load(node, config);
    _launcher_config_hooks_iat_load(node, config);
}

static void _launcher_config_debug_load(
    const core_property_node_t *node, struct launcher_debug_config *config)
{
    core_property_node_result_t result;

    log_assert(node);
    log_assert(config);

    result = core_property_node_ext_bool_read_or_default(
        node, "debug/remote_debugger", &config->remote_debugger, false);
    core_property_node_fatal_on_error(result);

    result = core_property_node_ext_bool_read_or_default(
        node,
        "debug/log_property_configs",
        &config->log_property_configs,
        false);
    core_property_node_fatal_on_error(result);
}

void launcher_config_init(struct launcher_config *config)
{
    log_assert(config);

    memset(config->bootstrap.selector, 0, sizeof(config->bootstrap.selector));
    config->bootstrap.property = NULL;

    config->avs.property = NULL;

    config->ea3_ident.property = NULL;

    config->eamuse.property = NULL;

    memset(&config->hook, 0, sizeof(config->hook));

    config->debug.remote_debugger = false;
    config->debug.log_property_configs = false;
}

void launcher_config_load(
    const core_property_t *property, struct launcher_config *config)
{
    core_property_node_t root_node;
    core_property_node_t node;
    core_property_node_result_t result;
    core_property_result_t result_prop;

    log_assert(property);
    log_assert(config);

    result_prop = core_property_root_node_get(property, &root_node);
    core_property_fatal_on_error(result_prop);

    result = core_property_node_search(&root_node, "version", &node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        NODE_MISSING_FATAL("version");
    } else {
        core_property_node_fatal_on_error(result);
    }

    result = core_property_node_u32_read(&node, &config->version);

    core_property_node_fatal_on_error(result);

    if (config->version != 1) {
        log_fatal(
            "Unsupported version of launcher configuration: %d",
            config->version);
    }

    result = core_property_node_search(&root_node, "bootstrap", &node);

    if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
        NODE_MISSING_FATAL("bootstrap");
    } else {
        core_property_node_fatal_on_error(result);
    }

    _launcher_config_bootstrap_load(&node, &config->bootstrap);

    result = core_property_node_search(&root_node, "avs", &node);

    if (!CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        config->avs.property =
            _launcher_config_layered_config_nodes_load(&node);
    } else {
        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            result_prop = core_property_str_load(
                "<config></config>", &config->avs.property);
            core_property_fatal_on_error(result_prop);
        } else {
            core_property_node_fatal_on_error(result);
        }
    }

    result = core_property_node_search(&root_node, "ea3_ident", &node);

    if (!CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        config->ea3_ident.property =
            _launcher_config_layered_config_nodes_load(&node);
    } else {
        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            result_prop = core_property_str_load(
                "<ea3_conf></ea3_conf>", &config->ea3_ident.property);
            core_property_fatal_on_error(result_prop);
        } else {
            core_property_node_fatal_on_error(result);
        }
    }

    result = core_property_node_search(&root_node, "eamuse", &node);

    if (!CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        config->eamuse.property =
            _launcher_config_layered_config_nodes_load(&node);
    } else {
        if (result == CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            result_prop =
                core_property_str_load("<ea3></ea3>", &config->eamuse.property);
            core_property_fatal_on_error(result_prop);
        } else {
            core_property_node_fatal_on_error(result);
        }
    }

    result = core_property_node_search(&root_node, "hook", &node);

    if (!CORE_PROPERTY_NODE_RESULT_IS_ERROR(result)) {
        _launcher_config_hooks_load(&node, &config->hook);
    } else {
        if (result != CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND) {
            core_property_node_fatal_on_error(result);
        }
    }
    // No defaults

    _launcher_config_debug_load(&root_node, &config->debug);
}

bool launcher_config_hooks_hook_add(
    struct launcher_config *config, const char *path)
{
    int i;
    core_property_result_t result;

    log_assert(config);
    log_assert(path);

    i = 0;

    while (i < LAUNCHER_CONFIG_MAX_HOOKS) {
        if (!launcher_config_hooks_hook_available(&config->hook.hook[i])) {
            break;
        }

        i++;
    }

    if (i >= LAUNCHER_CONFIG_MAX_HOOKS) {
        return false;
    }

    config->hook.hook[i].enable = true;
    str_cpy(config->hook.hook[i].path, sizeof(config->hook.hook[i].path), path);
    result = core_property_str_load(
        "<hook_conf><version __type=\"u32\">1</version></hook_conf>",
        &config->hook.hook[i].property);
    core_property_fatal_on_error(result);

    return true;
}

bool launcher_config_hooks_hook_available(
    const struct launcher_hook_config_hook *config)
{
    log_assert(config);

    return strlen(config->path) > 0;
}

bool launcher_config_hooks_pre_avs_hook_available(
    const struct launcher_hook_config_pre_avs *config)
{
    log_assert(config);

    return strlen(config->path) > 0;
}

bool launcher_config_hooks_iat_hook_available(
    const struct launcher_hook_config_iat *config)
{
    log_assert(config);

    return strlen(config->path) > 0;
}

void launcher_config_fini(struct launcher_config *config)
{
    int i;

    log_assert(config);

    core_property_free(&config->bootstrap.property);

    if (config->avs.property) {
        core_property_free(&config->avs.property);
    }

    if (config->ea3_ident.property) {
        core_property_free(&config->ea3_ident.property);
    }

    if (config->eamuse.property) {
        core_property_free(&config->eamuse.property);
    }

    for (i = 0; i < LAUNCHER_CONFIG_MAX_HOOKS; i++) {
        if (config->hook.hook[i].property) {
            core_property_free(&config->hook.hook[i].property);
        }
    }

    for (i = 0; i < LAUNCHER_CONFIG_MAX_HOOKS; i++) {
        if (config->hook.pre_avs[i].property) {
            core_property_free(&config->hook.pre_avs[i].property);
        }
    }

    for (i = 0; i < LAUNCHER_CONFIG_MAX_HOOKS; i++) {
        if (config->hook.iat[i].property) {
            core_property_free(&config->hook.iat[i].property);
        }
    }
}
#define LOG_MODULE "launcher-config"

#include <string.h>

#include "core/log.h"

#include "imports/avs.h"

#include "launcher/launcher-config.h"
#include "launcher/property-util.h"

#include "util/mem.h"
#include "util/str.h"

// clang-format off
PSMAP_BEGIN(launcher_debug_psmap)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct launcher_debug_config, remote_debugger,
    "debug/remote_debugger", false)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct launcher_debug_config, log_property_configs,
    "debug/log_property_configs", false)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct launcher_debug_config, procmon_file,
    "debug/procmon/file", false)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct launcher_debug_config, procmon_module,
    "debug/procmon/module", false)
PSMAP_OPTIONAL(PSMAP_TYPE_BOOL, struct launcher_debug_config, procmon_thread,
    "debug/procmon/thread", false)
PSMAP_END
// clang-format on

#define ROOT_NODE "/launcher"
#define MAX_LAYER_CONFIG_NODES 8

#define NODE_MISSING_FATAL(subnode) \
    log_fatal("%s/%s: Node missing", ROOT_NODE, subnode);
#define NODE_LOADING_FATAL(subnode) \
    log_fatal("%s/%s: Node loading", ROOT_NODE, subnode);

static struct property *
_launcher_config_layered_config_nodes_load(struct property_node *node)
{
    char kind[64];
    char file[MAX_PATH];
    int res;
    int cnt;

    struct property_node *cur;
    struct property *config_property[MAX_LAYER_CONFIG_NODES];
    struct property *merged_property;

    log_assert(node);

    cnt = 0;
    cur = property_search(NULL, node, "config");

    while (cur) {
        if (cnt >= MAX_LAYER_CONFIG_NODES) {
            log_fatal(
                "Exceeding max supported config nodes for layering, max is %d",
                MAX_LAYER_CONFIG_NODES);
        }

        res = property_node_refer(
            NULL, cur, "kind@", PROPERTY_TYPE_ATTR, kind, sizeof(kind));

        if (res < 0) {
            log_fatal("Failed reading 'kind' attribute value of config node");
        }

        if (!strcmp(kind, "file")) {
            property_node_read(cur, PROPERTY_TYPE_STR, file, sizeof(file));

            config_property[cnt] = property_util_load(file);
        } else if (!strcmp(kind, "inline")) {
            // The nested child is the actual root of the inline, not the outer
            // <config> node
            cur = property_node_traversal(cur, TRAVERSE_FIRST_CHILD);

            config_property[cnt] = property_util_clone(cur);
        } else {
            log_fatal(
                "Unsupported 'kind' attribute value '%s' of config node", kind);
        }

        cnt++;
        cur = property_node_traversal(cur, TRAVERSE_NEXT_SEARCH_RESULT);
    }

    if (cnt == 0) {
        return NULL;
    }

    merged_property =
        property_util_merge((struct property **) &config_property[0], cnt);

    for (int i = 0; i < cnt; i++) {
        property_util_free(config_property[i]);
    }

    return merged_property;
}

static void _launcher_config_hook_dlls_parse(
    struct property_node *node,
    const char *node_path,
    char dlls[LAUNCHER_CONFIG_MAX_HOOK_DLL][MAX_PATH])
{
    int cnt;
    struct property_node *cur;

    cnt = 0;
    cur = property_search(NULL, node, node_path);

    while (cur) {
        if (cnt >= LAUNCHER_CONFIG_MAX_HOOK_DLL) {
            log_warning(
                "Currently not supporting more than %d dlls, skipping "
                "remaining",
                cnt);
            break;
        }

        property_node_read(cur, PROPERTY_TYPE_STR, dlls[cnt], MAX_PATH);

        cnt++;
        cur = property_node_traversal(cur, TRAVERSE_NEXT_SEARCH_RESULT);
    }
}

static void _launcher_config_bootstrap_load(
    struct property_node *node, struct launcher_bootstrap_config *config)
{
    int res;

    log_assert(node);
    log_assert(config);

    res = property_node_refer(
        NULL,
        node,
        "selector",
        PROPERTY_TYPE_STR,
        config->selector,
        sizeof(config->selector));

    if (res < 0) {
        NODE_MISSING_FATAL("bootstrap/selector");
    }

    config->property = _launcher_config_layered_config_nodes_load(node);

    if (config->property == NULL) {
        NODE_MISSING_FATAL("bootstrap/config");
    }
}

static void _launcher_config_hook_load(
    struct property_node *node, struct launcher_hook_config *config)
{
    log_assert(node);
    log_assert(config);

    _launcher_config_hook_dlls_parse(node, "hook_dlls/dll", config->hook_dlls);
    _launcher_config_hook_dlls_parse(
        node, "before_hook_dlls/dll", config->before_hook_dlls);
    _launcher_config_hook_dlls_parse(
        node, "iat_hook_dlls/dll", config->iat_hook_dlls);
}

static void _launcher_config_debug_load(
    struct property_node *node, struct launcher_debug_config *config)
{
    log_assert(node);
    log_assert(config);

    if (!property_psmap_import(NULL, node, config, launcher_debug_psmap)) {
        NODE_LOADING_FATAL("debug");
    }
}

void launcher_config_init(struct launcher_config *config)
{
    log_assert(config);

    memset(config->bootstrap.selector, 0, sizeof(config->bootstrap.selector));
    config->bootstrap.property = NULL;

    config->avs.property = NULL;

    config->ea3_ident.property = NULL;

    config->eamuse.property = NULL;

    memset(config->hook.hook_dlls, 0, sizeof(config->hook.hook_dlls));
    memset(
        config->hook.before_hook_dlls,
        0,
        sizeof(config->hook.before_hook_dlls));
    memset(config->hook.iat_hook_dlls, 0, sizeof(config->hook.iat_hook_dlls));

    config->debug.remote_debugger = false;
    config->debug.log_property_configs = false;
    config->debug.procmon_file = false;
    config->debug.procmon_module = false;
    config->debug.procmon_thread = false;
}

void launcher_config_load(
    struct property *property, struct launcher_config *config)
{
    struct property_node *root_node;
    struct property_node *node;
    avs_error error;

    log_assert(property);
    log_assert(config);

    root_node = property_search(property, NULL, ROOT_NODE);

    if (root_node == NULL) {
        NODE_MISSING_FATAL("");
    }

    error = property_node_refer(
        NULL,
        root_node,
        "version@",
        PROPERTY_TYPE_ATTR,
        &config->version,
        sizeof(uint32_t));

    if (AVS_IS_ERROR(error)) {
        log_fatal("Missing version attribute on root node");
    }

    // if (config->version != 1) {
    //     log_fatal("Unsupported version of launcher configuration: %d",
    //     config->version);
    // }

    node = property_search(NULL, root_node, "bootstrap");

    if (node == NULL) {
        NODE_MISSING_FATAL("bootstrap");
    }

    _launcher_config_bootstrap_load(node, &config->bootstrap);

    node = property_search(NULL, root_node, "avs");

    if (node) {
        config->avs.property = _launcher_config_layered_config_nodes_load(node);
    }

    node = property_search(NULL, root_node, "ea3_ident");

    if (node) {
        config->ea3_ident.property =
            _launcher_config_layered_config_nodes_load(node);
    }

    node = property_search(NULL, root_node, "eamuse");

    if (node) {
        config->eamuse.property =
            _launcher_config_layered_config_nodes_load(node);
    }

    node = property_search(NULL, root_node, "hook");

    if (node) {
        _launcher_config_hook_load(node, &config->hook);
    }

    _launcher_config_debug_load(root_node, &config->debug);
}

bool launcher_config_add_hook_dll(
    struct launcher_config *config, const char *path)
{
    int i;

    log_assert(config);
    log_assert(path);

    i = 0;

    while (i < LAUNCHER_CONFIG_MAX_HOOK_DLL) {
        if (strlen(config->hook.hook_dlls[i]) == 0) {
            break;
        }

        i++;
    }

    if (i >= LAUNCHER_CONFIG_MAX_HOOK_DLL) {
        return false;
    }

    str_cpy(config->hook.hook_dlls[i], sizeof(config->hook.hook_dlls[i]), path);

    return true;
}

bool launcher_config_add_before_hook_dll(
    struct launcher_config *config, const char *path)
{
    int i;

    log_assert(config);
    log_assert(path);

    i = 0;

    while (i < LAUNCHER_CONFIG_MAX_HOOK_DLL) {
        if (strlen(config->hook.before_hook_dlls[i]) == 0) {
            break;
        }

        i++;
    }

    if (i >= LAUNCHER_CONFIG_MAX_HOOK_DLL) {
        return false;
    }

    str_cpy(
        config->hook.before_hook_dlls[i],
        sizeof(config->hook.before_hook_dlls[i]),
        path);

    return true;
}

bool launcher_config_add_iat_hook_dll(
    struct launcher_config *config, const char *path)
{
    int i;

    log_assert(config);
    log_assert(path);

    i = 0;

    while (i < LAUNCHER_CONFIG_MAX_HOOK_DLL) {
        if (strlen(config->hook.iat_hook_dlls[i]) == 0) {
            break;
        }

        i++;
    }

    if (i >= LAUNCHER_CONFIG_MAX_HOOK_DLL) {
        return false;
    }

    str_cpy(
        config->hook.iat_hook_dlls[i],
        sizeof(config->hook.iat_hook_dlls[i]),
        path);

    return true;
}

void launcher_config_fini(struct launcher_config *config)
{
    log_assert(config);

    property_util_free(config->bootstrap.property);

    if (config->avs.property) {
        property_util_free(config->avs.property);
    }

    if (config->ea3_ident.property) {
        property_util_free(config->ea3_ident.property);
    }

    if (config->eamuse.property) {
        property_util_free(config->eamuse.property);
    }
}
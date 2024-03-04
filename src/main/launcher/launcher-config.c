#define LOG_MODULE "launcher-config"

#include <string.h>

#include "avs-util/error.h"

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

            config_property[cnt] = property_util_node_extract(cur);
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

    merged_property = property_util_many_merge(config_property, cnt);

    for (int i = 0; i < cnt; i++) {
        property_util_free(config_property[i]);
    }

    return merged_property;
}

static void _launcher_config_iat_hook_dlls_parse(
    struct property_node *node,
    const char *node_path,
    struct launcher_hook_iat_config iat_hook_dlls[LAUNCHER_CONFIG_MAX_HOOKS])
{
    int cnt;
    struct property_node *cur;
    struct property_node *child;
    avs_error error;

    cnt = 0;
    cur = property_search(NULL, node, node_path);

    while (cur) {
        if (cnt >= LAUNCHER_CONFIG_MAX_HOOKS) {
            log_warning(
                "Currently not supporting more than %d dlls, skipping "
                "remaining",
                cnt);
            break;
        }

        child = property_search(NULL, cur, "source");

        if (!child) {
            log_fatal("Missing 'source' child node on iat hook property");
        }

        error = property_node_read(cur, PROPERTY_TYPE_STR, iat_hook_dlls[cnt].source, sizeof(iat_hook_dlls[cnt].source));

        if (AVS_IS_ERROR(error)) {
            log_fatal("Reading 'source' child node of iat hook property failed: %s", avs_util_error_str(error));
        }

        child = property_search(NULL, cur, "replace");

        if (!child) {
            log_fatal("Missing 'replace' child node on iat hook property");
        }

        error = property_node_read(cur, PROPERTY_TYPE_STR, iat_hook_dlls[cnt].replace, sizeof(iat_hook_dlls[cnt].replace));

        if (AVS_IS_ERROR(error)) {
            log_fatal("Reading 'replace' child node of iat hook property failed: %s", avs_util_error_str(error));
        }

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

static void _launcher_config_hooks_load(
    struct property_node *node, struct launcher_hooks_config *config)
{
    log_assert(node);
    log_assert(config);

    int cnt;
    struct property_node *cur;
    avs_error error;

    cnt = 0;
    cur = property_search(NULL, node, "hook");

    while (cur) {
        if (cnt >= LAUNCHER_CONFIG_MAX_HOOKS) {
            log_warning(
                "Currently not supporting more than %d hooks, skipping "
                "remaining",
                cnt);
            break;
        }
        
        error = property_node_read(cur, PROPERTY_TYPE_BOOL, &config->hook[cnt].enable);

        if (AVS_IS_ERROR(error)) {
            log_fatal("Reading 'enable' node of hook entry failed: %s", avs_util_error_str(error));
        }

        error = property_node_read(cur, PROPERTY_TYPE_STR, &config->hook[cnt].path);

        if (AVS_IS_ERROR(error)) {
            log_fatal("Reading 'path' node of hook entry failed: %s", avs_util_error_str(error));
        }

        config->hook[cnt].property = _launcher_config_layered_config_nodes_load(cur);        

        // Default empty config
        if (!config->hook[cnt].property) {
            config->hook[cnt].property = property_util_cstring_load("<hook><version __type\"u32\">1</version></hook>");
        }

        cnt++;
        cur = property_node_traversal(cur, TRAVERSE_NEXT_SEARCH_RESULT);
    }
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

    memset(config->hook.hooks, 0, sizeof(config->hook.hooks));

    config->debug.remote_debugger = false;
    config->debug.log_property_configs = false;
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

    node = property_search(NULL, root_node, "version");

    if (!node) {
        NODE_MISSING_FATAL("version");
    }

    error = property_node_read(node, PROPERTY_TYPE_U32, &config->version, sizeof(uint32_t));

    if (AVS_IS_ERROR(error)) {
        log_fatal("Reading 'version' node failed: %s", avs_util_error_str(error));
    }

    if (config->version != 1) {
        log_fatal("Unsupported version of launcher configuration: %d",
            config->version);
    }

    node = property_search(NULL, root_node, "bootstrap");

    if (node == NULL) {
        NODE_MISSING_FATAL("bootstrap");
    }

    _launcher_config_bootstrap_load(node, &config->bootstrap);

    node = property_search(NULL, root_node, "avs");

    if (node) {
        config->avs.property = _launcher_config_layered_config_nodes_load(node);
    } else {
        config->avs.property = property_util_cstring_load("<config></config>");
    }

    node = property_search(NULL, root_node, "ea3_ident");

    if (node) {
        config->ea3_ident.property =
            _launcher_config_layered_config_nodes_load(node);
    } else {
        config->ea3_ident.property = property_util_cstring_load("<ea3_conf></ea3_conf>");
    }

    node = property_search(NULL, root_node, "eamuse");

    if (node) {
        config->eamuse.property =
            _launcher_config_layered_config_nodes_load(node);
    } else {
        config->eamuse.property = property_util_cstring_load("<ea3></ea3>");
    }

    node = property_search(NULL, root_node, "hooks");

    if (node) {
        _launcher_config_hooks_load(node, &config->hooks);
    }
    // No defaults

    _launcher_config_debug_load(root_node, &config->debug);
}

bool launcher_config_hooks_hook_add(
    struct launcher_config *config, const char *path)
{
    int i;

    log_assert(config);
    log_assert(path);

    i = 0;

    while (i < LAUNCHER_CONFIG_MAX_HOOKS) {
        if (strlen(config->hooks.hook[i].path) == 0) {
            break;
        }

        i++;
    }

    if (i >= LAUNCHER_CONFIG_MAX_HOOKS) {
        return false;
    }

    config->hooks.hook[i].enable = true;
    str_cpy(config->hooks.hook[i].path, sizeof(config->hooks.hook[i].path), path);
    config->hooks.hook[i].property = property_util_cstring_load("<hook_conf><version __type=\"u32\">1</version></hook_conf>");

    return true;
}

bool launcher_config_hooks_available(const struct launcher_hooks_config *config)
{
    log_assert(config);

    return strlen(config->hook[0].path) > 0;
}

void launcher_config_fini(struct launcher_config *config)
{
    int i;

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

    for (i = 0; i < LAUNCHER_CONFIG_MAX_HOOKS; i++) {
        if (config->hooks.hook[i].property) {
            property_util_free(config->hooks.hook[i].property);
        }
    }
}
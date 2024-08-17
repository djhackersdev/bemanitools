#ifndef INJECT_HOOKS_CONFIG_H
#define INJECT_HOOKS_CONFIG_H

#include <windows.h>

#include <stdbool.h>

#include "core/property.h"
#include "core/property-node.h"

#define HOOKS_CONFIG_MAX_HOOKS 16

typedef struct hooks_config {
    struct hooks_hook_config {
        bool enable;
        char path[MAX_PATH];
        core_property_t *config;
    } hooks[HOOKS_CONFIG_MAX_HOOKS];

    struct hooks_iat_config {
        bool enable;
        char source_name[MAX_PATH];
        char path[MAX_PATH];
        core_property_t *config;
    } iats[HOOKS_CONFIG_MAX_HOOKS];
} hooks_config_t;

void hooks_config_init(hooks_config_t *config);

void hooks_config_load(
    const core_property_node_t *node, hooks_config_t *config);

bool hooks_config_hook_is_valid(const struct hooks_hook_config *hook);

bool hooks_config_iat_is_valid(const struct hooks_iat_config *hook);

void hooks_config_fini(hooks_config_t *config);

#endif
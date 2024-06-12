#ifndef LAUNCHER_CONFIG_H
#define LAUNCHER_CONFIG_H

#include <windows.h>

#include "core/property.h"

#define LAUNCHER_CONFIG_MAX_HOOKS 16

struct launcher_config {
    uint32_t version;

    struct launcher_bootstrap_config {
        char selector[128];
        core_property_t *property;
    } bootstrap;

    struct launcher_avs_config {
        core_property_t *property;
    } avs;

    struct launcher_ea3_ident_config {
        core_property_t *property;
    } ea3_ident;

    struct launcher_eamuse_config {
        core_property_t *property;
    } eamuse;

    struct launcher_hook_config {
        struct launcher_hook_config_hook {
            bool enable;
            char path[MAX_PATH];
            core_property_t *property;
        } hook[LAUNCHER_CONFIG_MAX_HOOKS];

        struct launcher_hook_config_pre_avs {
            bool enable;
            char path[MAX_PATH];
            core_property_t *property;
        } pre_avs[LAUNCHER_CONFIG_MAX_HOOKS];

        struct launcher_hook_config_iat {
            bool enable;
            char source_name[MAX_PATH];
            char path[MAX_PATH];
            core_property_t *property;
        } iat[LAUNCHER_CONFIG_MAX_HOOKS];
    } hook;

    struct launcher_debug_config {
        bool remote_debugger;
        bool log_property_configs;
        bool procmon_file;
        bool procmon_module;
        bool procmon_thread;
    } debug;
};

void launcher_config_init(struct launcher_config *config);

void launcher_config_load(
    const core_property_t *property, struct launcher_config *config);

bool launcher_config_hooks_hook_add(
    struct launcher_config *config, const char *path);

bool launcher_config_hooks_hook_available(
    const struct launcher_hook_config_hook *config);

bool launcher_config_hooks_pre_avs_hook_available(
    const struct launcher_hook_config_pre_avs *config);

bool launcher_config_hooks_iat_hook_available(
    const struct launcher_hook_config_iat *config);

void launcher_config_fini(struct launcher_config *config);

#endif
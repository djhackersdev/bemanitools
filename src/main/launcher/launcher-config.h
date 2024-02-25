#ifndef LAUNCHER_CONFIG_H
#define LAUNCHER_CONFIG_H

#include <windows.h>

#include "util/array.h"

#define LAUNCHER_CONFIG_MAX_HOOK_DLL 16

struct launcher_config {
    uint32_t version;

    struct launcher_bootstrap_config {
        char selector[128];
        struct property *property;
    } bootstrap;

    struct launcher_avs_config {
        struct property *property;
    } avs;

    struct launcher_ea3_ident_config {
        struct property *property;
    } ea3_ident;

    struct launcher_eamuse_config {
        struct property *property;
    } eamuse;

    struct launcher_hook_config {
        char hook_dlls[LAUNCHER_CONFIG_MAX_HOOK_DLL][MAX_PATH];
        char before_hook_dlls[LAUNCHER_CONFIG_MAX_HOOK_DLL][MAX_PATH];
        char iat_hook_dlls[LAUNCHER_CONFIG_MAX_HOOK_DLL][MAX_PATH];
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
    struct property *property, struct launcher_config *config);

bool launcher_config_add_hook_dll(
    struct launcher_config *config, const char *path);
bool launcher_config_add_before_hook_dll(
    struct launcher_config *config, const char *path);
bool launcher_config_add_iat_hook_dll(
    struct launcher_config *config, const char *path);

void launcher_config_fini(struct launcher_config *config);

#endif
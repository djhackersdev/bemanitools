#ifndef LAUNCHER_HOOKS_H
#define LAUNCHER_HOOKS_H

#include <windows.h>

#include "core/property-node.h"

void hooks_init();

void hooks_hook_load(const char *path, const core_property_node_t *node);

void hooks_core_config_api_set();

void hooks_core_log_api_set();

void hooks_core_thread_api_set();

void hooks_pre_avs_init();

void hooks_iat_apply(HMODULE game_module);

void hooks_main_init(HMODULE game_module);

void hooks_main_fini();

void hooks_fini();

#endif
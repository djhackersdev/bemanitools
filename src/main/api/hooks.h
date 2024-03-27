#ifndef API_HOOKS_H
#define API_HOOKS_H

#include <windows.h>

#include "api/hook.h"

#include "core/property-node.h"

void api_hooks_init();

void api_hooks_hook_load(const char *path, const core_property_node_t *node);

void api_hooks_core_thread_impl_set();

void api_hooks_core_log_impl_set();

void api_hooks_core_config_impl_set();

void api_hooks_before_avs_init();

void api_hooks_iat_apply(HMODULE game_module);

void api_hooks_main_init(HMODULE game_module);

void api_hooks_main_fini();

void api_hooks_fini();

#endif
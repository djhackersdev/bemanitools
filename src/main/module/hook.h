#ifndef MODULE_HOOK_H
#define MODULE_HOOK_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include "api/hook.h"

typedef void (*bt_module_hook_api_get_t)(bt_hook_api_t *api);

typedef struct module_hook module_hook_t;

void module_hook_load(const char *path, module_hook_t **module);
const char *module_hook_path_get(const module_hook_t *module);
void module_hook_free(module_hook_t **module);

void module_hook_core_config_api_set(
    const module_hook_t *module, const bt_core_config_api_t *api);
void module_hook_core_log_api_set(
    const module_hook_t *module, const bt_core_log_api_t *api);
void module_hook_core_thread_api_set(
    const module_hook_t *module, const bt_core_thread_api_t *api);

void module_hook_api_get(const module_hook_t *module, bt_hook_api_t *api);

#endif
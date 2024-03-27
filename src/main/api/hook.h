#ifndef API_HOOK_H
#define API_HOOK_H

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct api_hook api_hook_t;

void api_hook_load(const char *path, api_hook_t *hook);
const char *api_hook_path_get(const api_hook_t *hook);
void api_hook_core_thread_impl_set(const api_hook_t *hook);
void api_hook_core_log_impl_set(const api_hook_t *hook);
void api_hook_core_config_impl_set(const api_hook_t *hook);
bool api_hook_pre_avs_init(
    const api_hook_t *hook, const core_config_t *config);
void api_hook_iat_apply(const api_hook_t *hook, HMODULE game_module);
bool api_hook_main_init(
    const api_hook_t *hook,
    HMODULE game_module,
    const core_config_t *config);
void api_hook_main_fini(const api_hook_t *hook);
void api_hook_free(api_hook_t *hook);

#endif
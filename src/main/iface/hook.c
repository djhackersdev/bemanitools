#define LOG_MODULE "bt-hook"

#include <string.h>

#include "api/hook.h"

#include "iface-core/log.h"
#include "iface/hook.h"

#include "util/mem.h"
#include "util/str.h"

struct bt_hook {
    bt_hook_api_t api;
    char name[512];
};

void bt_hook_init(const bt_hook_api_t *api, const char *name, bt_hook_t **hook)
{
    log_assert(api);
    log_assert(name);
    log_assert(hook);

    if (api->version == 1) {
        // all functions optional, no need for asserting them

        *hook = xmalloc(sizeof(bt_hook_t));

        memcpy(&(*hook)->api, api, sizeof(bt_hook_api_t));
        str_cpy((*hook)->name, sizeof((*hook)->name), name);

        log_misc("[%s] api v1 detected", (*hook)->name);
    } else {
        log_fatal("[%s] Unsupported API version: %d", name, api->version);
    }
}

void bt_hook_fini(bt_hook_t **hook)
{
    log_assert(hook);

    free(*hook);
    memset(*hook, 0, sizeof(bt_hook_t));
}

bool bt_hook_pre_avs_init(const bt_hook_t *hook, const bt_core_config_t *config)
{
    bool result;

    log_assert(hook);
    log_assert(config);
    log_assert(hook->api.version > 0);

    if (hook->api.v1.pre_avs_init) {
        log_misc("[%s] >>> pre_avs_init", hook->name);

        result = hook->api.v1.pre_avs_init(config);

        log_misc("[%s] <<< pre_avs_init: %d", hook->name, result);
    } else {
        result = true;
    }

    return result;
}

void bt_hook_iat_dll_name_get(const bt_hook_t *hook, char *buffer, size_t size)
{
    log_assert(hook);
    log_assert(hook->api.version > 0);
    log_assert(buffer);
    log_assert(size > 0);

    if (hook->api.v1.iat_dll_name_get) {
        log_misc("[%s] >>> iat_dll_name_get", hook->name);

        hook->api.v1.iat_dll_name_get(buffer, size);

        log_misc("[%s] <<< iat_dll_name_get: %s", hook->name, buffer);
    }
}

bool bt_hook_main_init(
    const bt_hook_t *hook, HMODULE game_module, const bt_core_config_t *config)
{
    bool result;

    log_assert(hook);
    log_assert(game_module);
    log_assert(hook->api.version > 0);
    log_assert(config);

    if (hook->api.v1.main_init) {
        log_misc("[%s] >>> main_init", hook->name);

        result = hook->api.v1.main_init(game_module, config);

        log_misc("[%s] <<< main_init: %d", hook->name, result);
    } else {
        result = true;
    }

    return result;
}

void bt_hook_main_fini(const bt_hook_t *hook)
{
    log_assert(hook);
    log_assert(hook->api.version > 0);

    if (hook->api.v1.main_fini) {
        log_misc("[%s] >>> main_fini", hook->name);

        hook->api.v1.main_fini();

        log_misc("[%s] <<< main_fini", hook->name);
    }
}

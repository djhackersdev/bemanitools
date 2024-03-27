#define LOG_MODULE "api-hooks"

#include "api/hook.h"

#include "core/config.h"
#include "core/log.h"
#include "core/property-node.h"
#include "core/thread.h"

#define MAX_HOOKS 64

typedef struct api_hooks_hook {
    api_hook_t hook;
    const core_property_node_t *node;
} api_hooks_hook_t;

static api_hooks_hook_t _api_hooks_hooks[MAX_HOOKS];
static uint32_t _api_hooks_hooks_count;

void api_hooks_init()
{
    log_info("init");

    memset(_api_hooks_hooks, 0, sizeof(_api_hooks_hooks));
    _api_hooks_hooks_count = 0;

    log_misc("init done");
}

void api_hooks_hook_load(const char *path, struct property_node *node)
{
    log_assert(path);
    log_assert(node);

    log_misc("load: %s", path);

    if (_api_hooks_hooks_count >= MAX_HOOKS) {
        log_fatal("Cannot load more hooks, max supported capacity reached");
    }

    api_hook_load(path, &_api_hooks_hooks[_api_hooks_hooks_count].hook);
    _api_hooks_hooks[_api_hooks_hooks_count].node = node;
    _api_hooks_hooks_count++;
}

void api_hooks_core_thread_impl_set()
{
    uint32_t i;

    log_info("core_thread_impl_set");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        api_hook_core_thread_impl_set(&_api_hooks_hooks[i].hook);
    }

    log_misc("core_thread_impl_set done");
}

void api_hooks_core_log_impl_set()
{
    uint32_t i;

    log_info("core_log_impl_set");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        api_hook_core_log_impl_set(&_api_hooks_hooks[i].hook);
    }

    log_misc("core_log_impl_set done");
}

void api_hooks_core_config_impl_set()
{
    uint32_t i;

    log_info("core_config_impl_set");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        api_hook_core_config_impl_set(&_api_hooks_hooks[i].hook);
    }

    log_misc("core_config_impl_set done");
}

void api_hooks_pre_avs_init()
{
    uint32_t i;
    bool result;

    log_info("pre_avs_init");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        result = api_hook_pre_avs_init(
            &_api_hooks_hooks[i].hook, _api_hooks_hooks[i].node);

        if (!result) {
            log_fatal(
                "%s: pre avs initializing hook failed",
                bt_hook_path_get(&_api_hooks_hooks[i].hook));
        }
    }

    log_misc("pre_avs_init done");
}

void api_hooks_iat_apply(HMODULE game_module)
{
    uint32_t i;

    log_info("iat_apply");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        api_hook_iat_apply(&_api_hooks_hooks[i].hook, game_module);
    }

    log_misc("iat_apply done");
}

void api_hooks_main_init(HMODULE game_module)
{
    uint32_t i;
    bool result;

    log_assert(game_module);

    log_info("main_init");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        result = api_hook_main_init(
            &_api_hooks_hooks[i].hook, game_module, _api_hooks_hooks[i].node);

        if (!result) {
            log_fatal(
                "%s: Initializing hook failed",
                api_hook_path_get(&_api_hooks_hooks[i].hook));
        }
    }

    log_misc("main_init done");
}

void api_hooks_main_fini()
{
    uint32_t i;

    log_info("fini");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        api_hook_main_fini_invoke(&_api_hooks_hooks[i].hook);
    }

    log_misc("fini done");
}

void api_hooks_fini()
{
    uint32_t i;

    log_info("fini");

    for (i = 0; i < _api_hooks_hooks_count; i++) {
        api_hook_free(&_api_hooks_hooks[i].hook);
    }

    // node is just a reference to the property which is managed externally

    memset(_api_hooks_hooks, 0, sizeof(_api_hooks_hooks));

    log_misc("fini done");
}
#define LOG_MODULE "hooks"

#include "core/config-property-node.h"
#include "core/property-node.h"

#include "iface-core/config.h"
#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface/hook.h"

#include "module/hook.h"

#define MAX_HOOKS 64

typedef struct hooks_hook {
    module_hook_t *module;
    bt_hook_t *hook;
    const core_property_node_t *node;
} hooks_hook_t;

static hooks_hook_t _module_hooks[MAX_HOOKS];
static uint32_t _hooks_count;

void hooks_init()
{
    log_info("init");

    memset(_module_hooks, 0, sizeof(_module_hooks));
    _hooks_count = 0;

    log_misc("init done");
}

void hooks_hook_load(const char *path, const core_property_node_t *node)
{
    module_hook_t *module;
    bt_hook_api_t api;
    bt_hook_t *hook;

    log_assert(path);
    log_assert(node);

    log_misc("load: %s", path);

    if (_hooks_count >= MAX_HOOKS) {
        log_fatal("Cannot load more hooks, max supported capacity reached");
    }

    module_hook_load(path, &module);
    module_hook_api_get(module, &api);
    bt_hook_init(&api, path, &hook);

    _module_hooks[_hooks_count].module = module;
    _module_hooks[_hooks_count].hook = hook;
    _module_hooks[_hooks_count].node = node;
    _hooks_count++;
}

void hooks_core_config_api_set()
{
    bt_core_config_api_t api;
    uint32_t i;

    bt_core_config_api_get(&api);

    log_info("core_config_api_set");

    for (i = 0; i < _hooks_count; i++) {
        module_hook_core_config_api_set(_module_hooks[i].module, &api);
    }

    log_misc("core_config_api_set done");
}

void hooks_core_log_api_set()
{
    bt_core_log_api_t api;
    uint32_t i;

    bt_core_log_api_get(&api);

    log_info("core_log_api_set");

    for (i = 0; i < _hooks_count; i++) {
        module_hook_core_log_api_set(_module_hooks[i].module, &api);
    }

    log_misc("core_log_api_set done");
}

void hooks_core_thread_api_set()
{
    bt_core_thread_api_t api;
    uint32_t i;

    bt_core_thread_api_get(&api);

    log_info("core_thread_api_set");

    for (i = 0; i < _hooks_count; i++) {
        module_hook_core_thread_api_set(_module_hooks[i].module, &api);
    }

    log_misc("core_thread_api_set done");
}

void hooks_pre_avs_init()
{
    uint32_t i;
    bt_core_config_t *config;
    bool result;

    log_info("pre_avs_init");

    for (i = 0; i < _hooks_count; i++) {
        core_config_property_node_init(_module_hooks[i].node, &config);

        result = bt_hook_pre_avs_init(_module_hooks[i].hook, config);

        core_config_property_node_free(&config);

        if (!result) {
            log_fatal(
                "%s: pre avs initializing hook failed",
                module_hook_path_get(_module_hooks[i].module));
        }
    }

    log_misc("pre_avs_init done");
}

void hooks_iat_apply(HMODULE game_module)
{
    uint32_t i;
    char iat_dll_name[PATH_MAX];

    log_info("iat_apply");

    for (i = 0; i < _hooks_count; i++) {
        bt_hook_iat_dll_name_get(
            _module_hooks[i].hook, iat_dll_name, sizeof(iat_dll_name));

        // TODO fix iat hooking
        //     module_hook_iat_apply(game_module, "source_dll?", iat_dll_name);

        //     static void _module_hook_dll_iat(
        // HMODULE hModule, const char *source_dll, const char *iat_hook)

        //     (&, game_module);
    }

    log_misc("iat_apply done");
}

void hooks_main_init(HMODULE game_module)
{
    uint32_t i;
    bt_core_config_t *config;
    bool result;

    log_assert(game_module);

    log_info("main_init");

    for (i = 0; i < _hooks_count; i++) {
        core_config_property_node_init(_module_hooks[i].node, &config);

        result = bt_hook_main_init(_module_hooks[i].hook, game_module, config);

        core_config_property_node_free(&config);

        if (!result) {
            log_fatal(
                "%s: Initializing hook failed",
                module_hook_path_get(_module_hooks[i].module));
        }
    }

    log_misc("main_init done");
}

void hooks_main_fini()
{
    uint32_t i;

    log_info("main_fini");

    for (i = 0; i < _hooks_count; i++) {
        bt_hook_main_fini(_module_hooks[i].hook);
    }

    log_misc("main_fini done");
}

void hooks_fini()
{
    uint32_t i;

    log_info("fini");

    for (i = 0; i < _hooks_count; i++) {
        bt_hook_fini(&_module_hooks[i].hook);
        module_hook_free(&_module_hooks[i].module);
    }

    // node is just a reference to the property which is managed externally

    memset(_module_hooks, 0, sizeof(_module_hooks));

    log_misc("fini done");
}
#define LOG_MODULE "bt-hooks"

#include "bt/hook.h"

#include "core/log.h"
#include "core/thread.h"

// TODO move property-util to core and call property-ext
#include "launcher/property-util.h"

#define MAX_HOOKS 64

struct bt_hooks_hook {
    struct bt_hook hook;
    struct property_node *node;
};

static struct bt_hooks_hook _bt_hooks_hooks[MAX_HOOKS];
static uint32_t _bt_hooks_hooks_count;

void bt_hooks_init()
{
    log_info("init");

    memset(_bt_hooks_hooks, 0, sizeof(_bt_hooks_hooks));
    _bt_hooks_hooks_count = 0;

    log_misc("init done");
}

void bt_hooks_hook_load(const char *path, struct property_node *node)
{
    log_assert(path);
    log_assert(node);

    log_misc("load: %s", path);

    if (_bt_hooks_hooks_count >= MAX_HOOKS) {
        log_fatal("Cannot load more hooks, max supported capacity reached");
    }

    bt_hook_load(path, &_bt_hooks_hooks[_bt_hooks_hooks_count].hook);
    _bt_hooks_hooks[_bt_hooks_hooks_count].node = node;
    _bt_hooks_hooks_count++;
}

void bt_hooks_core_thread_impl_set_invoke()
{
    uint32_t i;

    log_info("core_thread_impl_set invoke");

    for (i = 0; i < _bt_hooks_hooks_count; i++) {
        bt_hook_core_thread_impl_set_invoke(
            _bt_hooks_hooks[i],
            core_thread_create_impl_get(),
            core_thread_join_impl_get(),
            core_thread_destroy_impl_get());
    }

    log_misc("core_thread_impl_set invoke done");
}

void bt_hooks_core_log_impl_set_invoke()
{
    uint32_t i;

    log_info("core_log_impl_set invoke");

    for (i = 0; i < _bt_hooks_hooks_count; i++) {
        bt_hook_core_log_impl_set_invoke(
            _bt_hooks_hooks[i],
            core_log_misc_impl_get(),
            core_log_info_impl_get(),
            core_log_warning_impl_get(),
            core_log_fatal_impl_get());
    }

    log_misc("core_log_impl_set invoke done");
}

void bt_hooks_before_avs_init_invoke()
{
    uint32_t i;
    bool result;
    struct property *tmp_empty_config_property;
    struct property_node *root_node;

    log_info("before_avs_init invoke");

    for (i = 0; i < _bt_hooks_hooks_count; i++) {
        result = bt_hook_before_avs_init_invoke(&_bt_hooks_hooks[i].hook, _bt_hooks_hooks[i].node);

        if (!result) {
            log_fatal("%s: before AVS initializing hook failed", bt_hook_path_get(_bt_hooks_hooks[i]));
        }
    }

    log_misc("before_avs_init invoke done");
}

void bt_hooks_iat_apply(HMODULE game_module)
{
    uint32_t i;

    log_info("Applying iat hook patches");

    for (i = 0; i < _bt_hooks_hooks_count; i++) {
        bt_hook_iat_apply(
            &_bt_hooks_hooks[i],
            game_module);
    }

    log_misc("Applying iat hook patches done");
}

void bt_hooks_main_init_invoke(HMODULE game_module)
{
    uint32_t i;
    bool result;
    struct property *tmp_empty_config_property;
    struct property_node *root_node;

    log_assert(game_module);

    log_info("main_init invoke");

    for (i = 0; i < _bt_hooks_hooks_count; i++) {
        result = bt_hook_main_init_invoke(&_bt_hooks_hooks[i].hook, game_module, _bt_hooks_hooks[i].node);

        if (!result) {
            log_fatal("%s: Initializing hook failed", bt_hook_path_get(_bt_hooks_hooks[i]));
        }
    }

    log_misc("main_init invoke done");
}

void bt_hooks_main_fini_invoke()
{
    uint32_t i;

    log_info("fini invoke");

    for (i = 0; i < _bt_hooks_hooks_count; i++) {
        bt_hook_fini_invoke(&_bt_hooks_hooks[i].hook);
    }

    log_misc("fini invoke done");
}

void bt_hooks_fini()
{
    uint32_t i;

    log_info("fini");

    for (i = 0; i < _bt_hooks_hooks_count; i++) {
        bt_hook_free(_bt_hooks_hooks[i]);
    }

    memset(_bt_hooks_hooks, 0, sizeof(_bt_hooks_hooks));

    log_misc("fini done");
}
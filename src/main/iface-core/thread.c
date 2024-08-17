#define LOG_MODULE "bt-core-thread"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "api/core/thread.h"

#include "iface-core/log.h"

#define BT_CORE_THREAD_ASSERT_IMPLEMENTED(func, name)              \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_core_thread_api_t _bt_core_thread_api;

static bool _bt_core_thread_api_is_valid()
{
    return _bt_core_thread_api.version > 0;
}

void bt_core_thread_api_set(const bt_core_thread_api_t *api)
{
    log_assert(api);

    if (_bt_core_thread_api_is_valid()) {
        log_info("Re-initialize");
    }

    if (api->version == 1) {
        BT_CORE_THREAD_ASSERT_IMPLEMENTED(api->v1.create, create);
        BT_CORE_THREAD_ASSERT_IMPLEMENTED(api->v1.join, join);
        BT_CORE_THREAD_ASSERT_IMPLEMENTED(api->v1.destroy, destroy);

        memcpy(&_bt_core_thread_api, api, sizeof(bt_core_thread_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_core_thread_api_get(bt_core_thread_api_t *api)
{
    log_assert(api);
    log_assert(_bt_core_thread_api_is_valid());

    memcpy(api, &_bt_core_thread_api, sizeof(bt_core_thread_api_t));
}

void bt_core_thread_api_clear()
{
    log_assert(_bt_core_thread_api_is_valid());

    memset(&_bt_core_thread_api, 0, sizeof(bt_core_thread_api_t));

    log_misc("api cleared");
}

const char *bt_core_thread_result_to_str(bt_core_thread_result_t result)
{
    switch (result) {
        case BT_CORE_THREAD_RESULT_SUCCESS:
            return "Success";
        case BT_CORE_THREAD_RESULT_ERROR_INTERNAL:
            return "Internal";
        default:
            return "Undefined error";
    }
}

void bt_core_thread_fatal_on_error(bt_core_thread_result_t result)
{
    switch (result) {
        case BT_CORE_THREAD_RESULT_SUCCESS:
            return;
        case BT_CORE_THREAD_RESULT_ERROR_INTERNAL:
        default:
            log_fatal(
                "Operation on thread failed: %s",
                bt_core_thread_result_to_str(result));
    }
}

bt_core_thread_result_t bt_core_thread_create(
    int (*proc)(void *),
    void *ctx,
    uint32_t stack_sz,
    unsigned int priority,
    bt_core_thread_id_t *thread_id)
{
    log_assert(_bt_core_thread_api_is_valid());
    log_assert(proc);
    log_assert(stack_sz > 0);
    log_assert(thread_id);

    return _bt_core_thread_api.v1.create(
        proc, ctx, stack_sz, priority, thread_id);
}

bt_core_thread_result_t
bt_core_thread_join(bt_core_thread_id_t thread_id, int *result)
{
    log_assert(_bt_core_thread_api_is_valid());

    return _bt_core_thread_api.v1.join(thread_id, result);
}

bt_core_thread_result_t bt_core_thread_destroy(bt_core_thread_id_t thread_id)
{
    log_assert(_bt_core_thread_api_is_valid());

    return _bt_core_thread_api.v1.destroy(thread_id);
}

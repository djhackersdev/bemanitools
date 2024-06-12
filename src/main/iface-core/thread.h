#ifndef BT_CORE_THREAD_H
#define BT_CORE_THREAD_H

#include <stdint.h>

#include "api/core/thread.h"

void bt_core_thread_api_set(const bt_core_thread_api_t *api);
void bt_core_thread_api_get(bt_core_thread_api_t *api);
void bt_core_thread_api_clear();

const char *bt_core_thread_result_to_str(bt_core_thread_result_t result);
void bt_core_thread_fatal_on_error(bt_core_thread_result_t result);

bt_core_thread_result_t bt_core_thread_create(
    int (*proc)(void *),
    void *ctx,
    uint32_t stack_sz,
    unsigned int priority,
    bt_core_thread_id_t *thread_id);
bt_core_thread_result_t
bt_core_thread_join(bt_core_thread_id_t thread_id, int *result);
bt_core_thread_result_t bt_core_thread_destroy(bt_core_thread_id_t thread_id);

#endif

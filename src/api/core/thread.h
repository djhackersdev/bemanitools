#ifndef BT_API_CORE_THREAD_H
#define BT_API_CORE_THREAD_H

#include <stdint.h>

#define BT_CORE_THREAD_RESULT_IS_ERROR(x) \
    (x > BT_CORE_THREAD_RESULT_SUCCESS)

typedef enum bt_core_thread_result {
    BT_CORE_THREAD_RESULT_SUCCESS = 0,
    BT_CORE_THREAD_RESULT_ERROR_INTERNAL = 1,
} bt_core_thread_result_t;

typedef int bt_core_thread_id_t;

typedef bt_core_thread_result_t (*bt_core_thread_create_t)(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, bt_core_thread_id_t *thread_id);
typedef bt_core_thread_result_t (*bt_core_thread_join_t)(bt_core_thread_id_t thread_id, int *result);
typedef bt_core_thread_result_t (*bt_core_thread_destroy_t)(bt_core_thread_id_t thread_id);

typedef struct bt_core_thread_api {
    uint16_t version;

    struct {
        bt_core_thread_create_t create;
        bt_core_thread_join_t join;
        bt_core_thread_destroy_t destroy;
    } v1;
} bt_core_thread_api_t;

#endif
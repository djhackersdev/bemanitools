#ifndef BTSDK_CORE_THREAD_H
#define BTSDK_CORE_THREAD_H

#include <stdint.h>

#include "btapi/core/thread.h"

void bt_core_thread_impl_set(const bt_core_thread_impl_t *impl);

const char *bt_core_thread_result_to_str(bt_core_thread_result_t result);
void bt_core_thread_fatal_on_error(bt_core_thread_result_t result);

/**
 * Create a thread
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * @param proc The function to run in a separate thread
 * @param ctx Additional data to pass to the function as a parameter
 * @param stack_sz The stack size to allocate for the thread in bytes
 * @param priority The thread's priority
 * @return The ID of the thread once created and started
 */
bt_core_thread_result_t bt_core_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, bt_core_thread_id_t *thread_id);

/**
 * Wait for a thread to finish
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * The caller of this function blocks until the thread has finished executing.
 *
 * @param thread_id ID of the thread to wait for
 * @param result Pointer to a variable to write the return value of the function
 *        the thread executed to
 */
void bt_core_thread_join(bt_core_thread_id_t thread_id, int *result);

/**
 * Destroy a thread
 *
 * Always use this interface in your application which hides the currently
 * configured implementation.
 *
 * The thread must have finished execution before calling this. It is advised
 * to make threads terminate their execution flow, join them and destroy.
 *
 * @param thread_id The ID of the thread to destroy.
 */
void bt_core_thread_destroy(bt_core_thread_id_t thread_id);

#endif

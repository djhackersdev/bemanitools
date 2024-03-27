#ifndef CORE_THREAD_H
#define CORE_THREAD_H

#include <stdint.h>

/**
 * The core thread API of bemanitools.
 *
 * This essentially reflects the AVS threading API and allows for swapping out
 * the backends with different implementations. Most games should have some
 * version of the AVS API available while some (legacy) games do not. These
 * can use a bemanitools private threading implementation by configuring it
 * in the bootstrapping process.
 */

typedef enum core_thread_result {
    CORE_THREAD_RESULT_SUCCESS = 0,
    CORE_THREAD_RESULT_ERROR_INTERNAL = 1,
} core_thread_result_t;

typedef int core_thread_id_t;

typedef core_thread_result_t (*core_thread_create_impl_t)(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, core_thread_id_t *thread_id);
typedef core_thread_result_t (*core_thread_join_impl_t)(core_thread_id_t thread_id, int *result);
typedef core_thread_result_t (*core_thread_destroy_impl_t)(core_thread_id_t thread_id);

typedef struct core_thread_impl {
    core_thread_create_impl_t create;
    core_thread_join_impl_t join;
    core_thread_destroy_impl_t destroy;
} core_thread_impl_t;

void core_thread_impl_set(const core_thread_impl_t *impl);
const core_thread_impl_t *core_thread_impl_get();

const char *core_thread_result_to_str(core_thread_result_t result);
void core_thread_fatal_on_error(core_thread_result_t result);

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
core_thread_result_t core_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority, core_thread_id_t *thread_id);

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
void core_thread_join(core_thread_id_t thread_id, int *result);

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
void core_thread_destroy(core_thread_id_t thread_id);

#endif

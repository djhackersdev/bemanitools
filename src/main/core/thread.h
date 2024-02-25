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
int core_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority);

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
void core_thread_join(int thread_id, int *result);

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
void core_thread_destroy(int thread_id);

typedef int (*core_thread_create_t)(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority);
typedef void (*core_thread_join_t)(int thread_id, int *result);
typedef void (*core_thread_destroy_t)(int thread_id);

typedef void (*core_thread_impl_set_t)(
    core_thread_create_t create,
    core_thread_join_t join,
    core_thread_destroy_t destroy);

/**
 * Configure the thread API implementations
 *
 * Advised to do this as early in your application/library module as possible
 * as calls to the getter functions below will return the currently configured
 * implementations.
 *
 * @param create Pointer to a function implementing thread creation
 * @param join Pointer to a function implementing joining of a thread
 * @param destroy Pointer to a function implementing destroying of a thread
 */
void core_thread_impl_set(
    core_thread_create_t create,
    core_thread_join_t join,
    core_thread_destroy_t destroy);

/**
 * Supporting function to inject/assign the currently set implementation
 * with the given setter function.
 *
 * @param impl_set Setter function to call with the currently configured thread
 * function implementations
 */
void core_thread_impl_assign(core_thread_impl_set_t impl_set);

/**
 * Get the currently configured implementation for thread_create
 *
 * @return Pointer to the currently configured implementation of the
 * thread_create function
 */
core_thread_create_t core_thread_create_impl_get();

/**
 * Get the currently configured implementation for thread_join
 *
 * @return Pointer to the currently configured implementation of the thread_join
 * function
 */
core_thread_join_t core_thread_join_impl_get();

/**
 * Get the currently configured implementation for thread_destroy
 *
 * @return Pointer to the currently configured implementation of the
 * thread_destroy function
 */
core_thread_destroy_t core_thread_destroy_impl_get();

#endif

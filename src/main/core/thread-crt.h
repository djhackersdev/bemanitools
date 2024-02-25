#ifndef CORE_THREAD_CRT_H
#define CORE_THREAD_CRT_H

#include <stdint.h>

/**
 * Thread API implementation using the C runtime API
 */

int core_thread_crt_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority);
void core_thread_crt_join(int thread_id, int *result);
void core_thread_crt_destroy(int thread_id);

#endif

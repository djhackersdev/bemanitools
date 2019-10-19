#ifndef UTIL_THREAD_H
#define UTIL_THREAD_H

#include <stdint.h>

#include "bemanitools/glue.h"

int crt_thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority);
void crt_thread_join(int thread_id, int *result);
void crt_thread_destroy(int thread_id);

void thread_api_init(
    thread_create_t create, thread_join_t join, thread_destroy_t destroy);
int thread_create(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority);
void thread_join(int thread_id, int *result);
void thread_destroy(int thread_id);

#endif

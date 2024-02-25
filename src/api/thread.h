#ifndef BEMANITOOLS_API_THREAD_H
#define BEMANITOOLS_API_THREAD_H

#include <stdint.h>

/* An API for spawning threads. This API is defined by libavs, although
   Bemanitools itself may supply compatible implementations of these functions
   to your DLL, depending on the context in which it runs.

   NOTE: You may only use the logging functions from a thread where Bemanitools
   calls you, or a thread that you create using this API. Failure to observe
   this restriction will cause the process to crash. This is a limitation of
   libavs itself, not Bemanitools. */

typedef int (*btapi_thread_create_t)(
    int (*proc)(void *), void *ctx, uint32_t stack_sz, unsigned int priority);
typedef void (*btapi_thread_join_t)(int thread_id, int *result);
typedef void (*btapi_thread_destroy_t)(int thread_id);

#endif

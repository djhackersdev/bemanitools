#ifndef BTAPI_HOOK_CORE_H
#define BTAPI_HOOK_CRE_H

#include "log.h"
#include "thread.h"

// Remark: these can actually be called multiple times by the backend if the backend
// implementation swapped
void btapi_hook_core_thread_impl_set(
    btapi_thread_create_t create,
    btapi_thread_join_t join,
    btapi_thread_destroy_t destroy);

// Remark: these can actually be called multiple times by the backend if the backend
// implementation swapped
void btapi_hook_core_log_impl_set(
    btapi_log_formatter_t misc,
    btapi_log_formatter_t info,
    btapi_log_formatter_t warning,
    btapi_log_formatter_t fatal);

#endif
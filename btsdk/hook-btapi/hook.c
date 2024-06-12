#include "api/hook.h"

void btapi_hook_thread_impl_set(
    btapi_thread_create_t create,
    btapi_thread_join_t join,
    btapi_thread_destroy_t destroy)
{
    // TODO some simple example how to use basic functionality of the API
}

void btapi_hook_log_impl_set(
    btapi_log_formatter_t misc,
    btapi_log_formatter_t info,
    btapi_log_formatter_t warning,
    btapi_log_formatter_t fatal)
{
    // TODO some simple example how to use basic functionality of the API
}

bool btapi_hook_init(struct property_node *config)
{
    // TODO some simple example how to use basic functionality of the API

    return true;
}

void btapi_hook_fini()
{
    // TODO some simple example how to use basic functionality of the API
}
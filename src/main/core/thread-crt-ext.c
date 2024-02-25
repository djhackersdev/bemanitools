#include "core/thread-crt.h"
#include "core/thread.h"

void core_thread_crt_ext_impl_set()
{
    core_thread_impl_set(
        core_thread_crt_create, core_thread_crt_join, core_thread_crt_destroy);
}
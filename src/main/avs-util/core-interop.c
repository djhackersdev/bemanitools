#include "core/log.h"
#include "core/thread.h"

#include "imports/avs.h"

void avs_util_core_interop_log_avs_impl_set()
{
    core_log_impl_set(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);
}

void avs_util_core_interop_thread_avs_impl_set()
{
    core_thread_impl_set(
        avs_thread_create, avs_thread_join, avs_thread_destroy);
}
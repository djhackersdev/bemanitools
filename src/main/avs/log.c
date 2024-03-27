#include "avs/log.h"

#include "imports/avs.h"

void avs_log_impl_get(core_log_impl_t *impl)
{
    log_assert(impl);

    impl->misc = log_body_misc;
    impl->info = log_body_info;
    impl->warning = log_body_warning;
    impl->fatal = log_body_fatal;
}
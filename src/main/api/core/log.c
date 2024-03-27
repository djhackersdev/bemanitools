#include <args.h>

#include "btapi/core/log.h"

#include "core/log.h"

void api_core_log_impl_get(bt_core_log_impl_t *impl)
{
    log_assert(impl);

    impl->misc = _core_log_impl.misc;
    impl->info = _core_log_impl.info;
    impl->warning = _core_log_impl.warning;
    impl->fatal = _core_log_impl.fatal;
}
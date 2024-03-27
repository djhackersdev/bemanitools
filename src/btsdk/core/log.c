#include <stdlib.h>

#include "core/log.h"

bt_core_log_impl_t _core_log_impl;

void bt_core_log_impl_set(const bt_core_log_impl_t *impl)
{
    log_assert(impl);

    if (_bt_core_log_impl.misc) {
        log_warning("Re-initialize");
    }

    memcpy(_bt_core_log_impl, impl, sizeof(bt_core_log_impl_t));
}
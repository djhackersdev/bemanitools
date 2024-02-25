#include <stdlib.h>

#include "core/log.h"

core_log_message_t _core_log_misc_impl;
core_log_message_t _core_log_info_impl;
core_log_message_t _core_log_warning_impl;
core_log_message_t _core_log_fatal_impl;

void core_log_impl_set(
    core_log_message_t misc,
    core_log_message_t info,
    core_log_message_t warning,
    core_log_message_t fatal)
{
    if (misc == NULL || info == NULL || warning == NULL || fatal == NULL) {
        abort();
    }

    _core_log_misc_impl = misc;
    _core_log_info_impl = info;
    _core_log_warning_impl = warning;
    _core_log_fatal_impl = fatal;
}

void core_log_impl_assign(core_log_impl_set_t impl_set)
{
    if (_core_log_misc_impl == NULL || _core_log_info_impl == NULL ||
        _core_log_warning_impl == NULL || _core_log_fatal_impl == NULL) {
        abort();
    }

    impl_set(
        _core_log_misc_impl,
        _core_log_info_impl,
        _core_log_warning_impl,
        _core_log_fatal_impl);
}

core_log_message_t core_log_misc_impl_get()
{
    if (_core_log_misc_impl == NULL) {
        abort();
    }

    return _core_log_misc_impl;
}

core_log_message_t core_log_info_impl_get()
{
    if (_core_log_info_impl == NULL) {
        abort();
    }

    return _core_log_info_impl;
}

core_log_message_t core_log_warning_impl_get()
{
    if (_core_log_warning_impl == NULL) {
        abort();
    }

    return _core_log_warning_impl;
}

core_log_message_t core_log_fatal_impl_get()
{
    if (_core_log_fatal_impl == NULL) {
        abort();
    }

    return _core_log_fatal_impl;
}
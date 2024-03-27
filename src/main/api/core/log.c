#include <args.h>

#include "btapi/core/log.h"

#include "core/log.h"

static void _api_core_log_misc(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    core_log_misc_va(module, fmt, args);

    va_end(args);
}

static void _api_core_log_info(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    core_log_info_va(module, fmt, args);

    va_end(args);
}

static void _api_core_log_warning(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    core_log_warning_va(module, fmt, args);

    va_end(args);
}

static void _api_core_log_fatal(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    core_log_fatal_va(module, fmt, args);

    va_end(args);
}

void api_core_log_impl_get(bt_core_log_impl_t *impl)
{
    log_assert(impl);

    impl->misc = _api_core_log_misc;
    impl->info = _api_core_log_info;
    impl->warning = _api_core_log_warning;
    impl->fatal = _api_core_log_fatal;
}
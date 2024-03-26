#include <stdlib.h>

#include "core/log.h"

#define CORE_LOG_ASSERT_IMPLEMENTED(func, name) \
    while (0) { \
        if (!func) { \
            log_fatal("Function %s not implemented", STRINGIFY(name)); \
        } \
    }

static core_log_impl_t _core_log_impl;

void core_log_impl_set(const core_log_impl_t *impl)
{
    log_assert(impl);

    if (_core_log_impl.misc) {
        log_warning("Re-initialize");
    }

    CORE_LOG_ASSERT_IMPLEMENTED(impl->misc, misc);
    CORE_LOG_ASSERT_IMPLEMENTED(impl->info, info);
    CORE_LOG_ASSERT_IMPLEMENTED(impl->warning, warning);
    CORE_LOG_ASSERT_IMPLEMENTED(impl->fatal, fatal);

    memcpy(_core_log_impl, impl, sizeof(core_log_impl_t));
}

const core_log_impl_t *core_log_impl_get()
{
    log_assert(_core_log_impl.misc);

    return &_core_log_impl;
}

void core_log_misc(const char *module, const char *fmt, ...)
{
    va_list args;

    log_assert(_core_log_impl.misc);
    log_assert(module);
    log_assert(fmt);

    va_start(args, fmt);

    _core_log_impl.misc(module, fmt, args);

    va_end(args);
}

void core_log_info(const char *module, const char *fmt, ...)
{
    va_list args;

    log_assert(_core_log_impl.misc);
    log_assert(module);
    log_assert(fmt);

    va_start(args, fmt);

    _core_log_impl.info(module, fmt, args);

    va_end(args);
}

void core_log_warning(const char *module, const char *fmt, ...)
{
    va_list args;

    log_assert(_core_log_impl.misc);
    log_assert(module);
    log_assert(fmt);

    va_start(args, fmt);

    _core_log_impl.warning(module, fmt, args);

    va_end(args);
}

void core_log_fatal(const char *module, const char *fmt, ...)
{
    va_list args;

    // don't assert arguments because log_assert depends on fatal -> avoid recursion

    va_start(args, fmt);

    _core_log_impl.fatal(module, fmt, args);

    va_end(args);
}

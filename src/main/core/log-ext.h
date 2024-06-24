#ifndef CORE_LOG_EXT_H
#define CORE_LOG_EXT_H

#include "iface-core/log.h"

#define log_fatal_on_win_last_error(...)                                        \
    if (GetLastError() != 0) {                                                  \
        _bt_core_log_api.v1.fatal(                                              \
            LOG_MODULE,                                                         \
            "%s:%d: function `%s'",                                             \
            __FILE__,                                                           \
            __LINE__,                                                           \
            __FUNCTION__);                                                      \
        _bt_core_log_api.v1.fatal(LOG_MODULE, __VA_ARGS__);                     \
        core_log_ext_win_last_error_log(LOG_MODULE, _bt_core_log_api.v1.fatal); \
        _bt_core_log_api.v1.fatal(LOG_MODULE, "");                              \
        abort();                                                                \
    }

void core_log_ext_win_last_error_log(const char *module, bt_core_log_message_t log_message);

#endif
#ifndef BT_CORE_LOG_H
#define BT_CORE_LOG_H

typedef void (*bt_core_log_message_t)(const char *module, const char *fmt, ...);

typedef struct bt_core_log_impl {
    bt_core_log_message_t misc;
    bt_core_log_message_t info;
    bt_core_log_message_t warning;
    bt_core_log_message_t fatal;
} bt_core_log_impl_t;

#endif

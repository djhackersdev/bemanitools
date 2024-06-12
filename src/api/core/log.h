#ifndef BT_API_CORE_LOG_H
#define BT_API_CORE_LOG_H

#include <stdint.h>

typedef void (*bt_core_log_message_t)(const char *module, const char *fmt, ...);

typedef struct bt_core_log_api {
    uint16_t version;

    struct {
        bt_core_log_message_t misc;
        bt_core_log_message_t info;
        bt_core_log_message_t warning;
        bt_core_log_message_t fatal;
    } v1;
} bt_core_log_api_t;

#endif
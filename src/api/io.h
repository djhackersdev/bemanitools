#ifndef BT_API_IO_H
#define BT_API_IO_H

#include <stdbool.h>
#include <stdint.h>

#include "api/core/config.h"

typedef bool (*bt_io_configure_t)(const bt_core_config_t *config);

typedef struct bt_io_api {
    uint16_t version;

    struct {
        // Optional
        bt_io_configure_t configure;
    } v1;
} bt_hook_api_t;

#endif
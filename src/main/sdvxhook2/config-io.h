#ifndef SDVXHOOK2_CONFIG_IO_H
#define SDVXHOOK2_CONFIG_IO_H

#include <stdbool.h>

#include "api/core/config.h"

typedef struct sdvxhook2_config_io {
    bool disable_card_reader_emu;
    bool disable_bio2_emu;
    bool disable_poll_limiter;
    bool force_headphones;
    bool disable_file_hooks;
    bool disable_power_hooks;
    bool disable_nvapi_hooks;
    bool com1_card_reader;
} sdvxhook2_config_io_t;

void sdvxhook2_config_io_get(
    const bt_core_config_t *config, sdvxhook2_config_io_t *config_out);

#endif
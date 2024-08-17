#ifndef IIDXHOOK_UTIL_CONFIG_IO_H
#define IIDXHOOK_UTIL_CONFIG_IO_H

#include <windows.h>

#include "iface-core/config.h"

typedef struct iidxhook_config_io {
    bool disable_card_reader_emu;
    bool disable_io_emu;
} iidxhook_config_io_t;

void iidxhook_util_config_io_get(
    const bt_core_config_t *config,
    iidxhook_config_io_t *config_io);

#endif
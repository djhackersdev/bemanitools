#ifndef IIDXHOOK8_CONFIG_IO_H
#define IIDXHOOK8_CONFIG_IO_H

#include <windows.h>

#include "cconfig/cconfig.h"

#include "iface-core/config.h"

struct iidxhook8_config_io {
    bool disable_card_reader_emu;
    bool disable_bio2_emu;
    bool disable_poll_limiter;
};

void iidxhook8_config_io_init(struct cconfig *config);

void iidxhook8_config_io_get(
    struct iidxhook8_config_io *config_io, struct cconfig *config);

void iidxhook8_config_io_get2(
    const bt_core_config_t *config,
    struct iidxhook8_config_io *config_io);

#endif
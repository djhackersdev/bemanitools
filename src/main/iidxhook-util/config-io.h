#ifndef IIDXHOOK_UTIL_CONFIG_IO_H
#define IIDXHOOK_UTIL_CONFIG_IO_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct iidxhook_config_io {
    bool disable_card_reader_emu;
    bool disable_io_emu;
};

void iidxhook_config_io_init(struct cconfig *config);

void iidxhook_config_io_get(
    struct iidxhook_config_io *config_io, struct cconfig *config);

#endif
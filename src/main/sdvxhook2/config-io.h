#ifndef SDVXHOOK2_CONFIG_IO_H
#define SDVXHOOK2_CONFIG_IO_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct sdvxhook2_config_io {
    bool disable_card_reader_emu;
    bool disable_bio2_emu;
    bool disable_poll_limiter;
};

void sdvxhook2_config_io_init(struct cconfig *config);

void sdvxhook2_config_io_get(
    struct sdvxhook2_config_io *config_io, struct cconfig *config);

#endif
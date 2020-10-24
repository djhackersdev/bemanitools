#ifndef IIDXHOOK9_CONFIG_IO_H
#define IIDXHOOK9_CONFIG_IO_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct iidxhook9_config_io {
    bool disable_card_reader_emu;
    bool disable_bio2_emu;
    bool disable_poll_limiter;
    bool lightning_mode;
    bool disable_cams;
    bool disable_file_hooks;
};

void iidxhook9_config_io_init(struct cconfig *config);

void iidxhook9_config_io_get(
    struct iidxhook9_config_io *config_io, struct cconfig *config);

#endif
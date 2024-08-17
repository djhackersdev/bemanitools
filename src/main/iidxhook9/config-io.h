#ifndef IIDXHOOK9_CONFIG_IO_H
#define IIDXHOOK9_CONFIG_IO_H

#include <stdbool.h>

struct iidxhook9_config_io {
    bool disable_card_reader_emu;
    bool disable_bio2_emu;
    bool disable_poll_limiter;
    bool lightning_mode;
    bool disable_cams;
    bool disable_file_hooks;
    float tt_multiplier;
};

void iidxhook9_config_io_get(
    const bt_core_config_t *config,
    struct iidxhook9_config_io *config_io);

#endif
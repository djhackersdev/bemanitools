#include "core/config-ext.h"

#include "iidxhook8/config-io.h"

void iidxhook8_config_io_get(
    const bt_core_config_t *config,
    struct iidxhook8_config_io *config_io)
{
    bt_core_config_ext_bool_get(config, "io/disable_card_reader_emu", &config_io->disable_card_reader_emu);
    bt_core_config_ext_bool_get(config, "io/disable_bio2_emu", &config_io->disable_bio2_emu);
    bt_core_config_ext_bool_get(config, "io/disable_poll_limiter", &config_io->disable_poll_limiter);
}
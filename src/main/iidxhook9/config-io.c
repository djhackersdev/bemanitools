#include "core/config-ext.h"

#include "iface-core/log.h"

#include "iidxhook9/config-io.h"

void iidxhook9_config_io_get(
    const bt_core_config_t *config,
    struct iidxhook9_config_io *config_io)
{
    bt_core_config_ext_bool_get(config, "io/disable_card_reader_emu", &config_io->disable_card_reader_emu);
    bt_core_config_ext_bool_get(config, "io/disable_bio2_emu", &config_io->disable_bio2_emu);
    bt_core_config_ext_bool_get(config, "io/disable_poll_limiter", &config_io->disable_poll_limiter);
    bt_core_config_ext_bool_get(config, "io/disable_cams", &config_io->disable_cams);
    bt_core_config_ext_bool_get(config, "io/disable_file_hooks", &config_io->disable_file_hooks);
    bt_core_config_ext_float_get(config, "io/tt_multiplier", &config_io->tt_multiplier);
}
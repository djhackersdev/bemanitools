#include "core/config-ext.h"

#include "iface-core/log.h"

#include "iidxhook-util/config-io.h"

void iidxhook_util_config_io_get(
    const bt_core_config_t *config,
    iidxhook_config_io_t *config_io)
{
    bt_core_config_ext_bool_get(config, "io/disable_card_reader_emu", &config_io->disable_card_reader_emu);
    bt_core_config_ext_bool_get(config, "io/disable_io_emu", &config_io->disable_io_emu);
}
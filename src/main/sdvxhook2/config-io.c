#include "core/config-ext.h"

#include "sdvxhook2/config-io.h"

void sdvxhook2_config_io_get(
    const bt_core_config_t *config, sdvxhook2_config_io_t *config_out)
{
    bt_core_config_ext_bool_get(config, "io/disable_card_reader_emu", &config_out->disable_card_reader_emu);
    bt_core_config_ext_bool_get(config, "io/disable_bio2_emu", &config_out->disable_bio2_emu);
    bt_core_config_ext_bool_get(config, "io/disable_poll_limiter", &config_out->disable_poll_limiter);
    bt_core_config_ext_bool_get(config, "io/force_headphones", &config_out->force_headphones);
    bt_core_config_ext_bool_get(config, "io/disable_file_hooks", &config_out->disable_file_hooks);
    bt_core_config_ext_bool_get(config, "io/disable_power_hooks", &config_out->disable_power_hooks);
    bt_core_config_ext_bool_get(config, "io/disable_nvapi_hooks", &config_out->disable_nvapi_hooks);
    bt_core_config_ext_bool_get(config, "io/com1_card_reader", &config_out->com1_card_reader);
}
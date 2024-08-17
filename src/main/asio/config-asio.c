#include "asio/config-asio.h"

void asiohook_config_asio_get(
    const bt_core_config_t *config,
    struct asiohook_config_asio *config_asio)
{
    bt_core_config_bool_get(config, "asio/force_asio", &config_asio->force_asio);
    bt_core_config_bool_get(config, "asio/force_wasapi", &config_asio->force_wasapi);
    bt_core_config_str_get(config, "asio/device_name", config_asio->replacement_name, sizeof(config_asio->replacement_name));
}
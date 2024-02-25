#include "cconfig/cconfig-util.h"

#include "core/log.h"

#include "asio/config-asio.h"

#define ASIOHOOK_CONFIG_IO_FORCE_ASIO_KEY "asio.force_asio"
#define ASIOHOOK_CONFIG_IO_FORCE_WASAPI_KEY "asio.force_wasapi"
#define ASIOHOOK_CONFIG_IO_ASIO_DEVICE_NAME_KEY "asio.device_name"

#define ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_ASIO_VALUE false
#define ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_WASAPI_VALUE false
#define ASIOHOOK_CONFIG_IO_DEFAULT_ASIO_DEVICE_NAME_VALUE "XONAR SOUND CARD(64)"

void asiohook_config_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        ASIOHOOK_CONFIG_IO_FORCE_ASIO_KEY,
        ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_ASIO_VALUE,
        "Force ASIO audio mode/device (if applicable)");

    cconfig_util_set_bool(
        config,
        ASIOHOOK_CONFIG_IO_FORCE_WASAPI_KEY,
        ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_WASAPI_VALUE,
        "Force WASAPI audio mode (if applicable)");

    cconfig_util_set_str(
        config,
        ASIOHOOK_CONFIG_IO_ASIO_DEVICE_NAME_KEY,
        ASIOHOOK_CONFIG_IO_DEFAULT_ASIO_DEVICE_NAME_VALUE,
        "The ASIO device name to use");
}

void asiohook_config_asio_get(
    struct asiohook_config_asio *config_asio, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            ASIOHOOK_CONFIG_IO_FORCE_ASIO_KEY,
            &config_asio->force_asio,
            ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_ASIO_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            ASIOHOOK_CONFIG_IO_FORCE_ASIO_KEY,
            ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_ASIO_VALUE);
    }
    if (!cconfig_util_get_bool(
            config,
            ASIOHOOK_CONFIG_IO_FORCE_WASAPI_KEY,
            &config_asio->force_wasapi,
            ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_WASAPI_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            ASIOHOOK_CONFIG_IO_FORCE_WASAPI_KEY,
            ASIOHOOK_CONFIG_IO_DEFAULT_FORCE_WASAPI_VALUE);
    }

    if (!cconfig_util_get_str(
            config,
            ASIOHOOK_CONFIG_IO_ASIO_DEVICE_NAME_KEY,
            config_asio->replacement_name,
            sizeof(config_asio->replacement_name) - 1,
            ASIOHOOK_CONFIG_IO_DEFAULT_ASIO_DEVICE_NAME_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            ASIOHOOK_CONFIG_IO_ASIO_DEVICE_NAME_KEY,
            ASIOHOOK_CONFIG_IO_DEFAULT_ASIO_DEVICE_NAME_VALUE);
    }
}

#include "cconfig/cconfig-util.h"

#include "sdvxio-kfca/config-kfca.h"

#include "util/log.h"

#define SDVXIO_KFCA_CONFIG_KFCA_PORT_KEY "kfca.port"
#define SDVXIO_KFCA_CONFIG_KFCA_BAUD_KEY "kfca.baud"

#define SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_PORT_VALUE "COM3"
#define SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_BAUD_VALUE 57600

void sdvxio_kfca_config_kfca_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        SDVXIO_KFCA_CONFIG_KFCA_PORT_KEY,
        SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_PORT_VALUE,
        "KFCA ACIO serial port");

    cconfig_util_set_int(
        config,
        SDVXIO_KFCA_CONFIG_KFCA_BAUD_KEY,
        SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_BAUD_VALUE,
        "KFCA ACIO bus baudrate (real devices expect 57600)");
}

void sdvxio_kfca_config_kfca_get(
    struct sdvxio_kfca_config_kfca *config_kfca, struct cconfig *config)
{
    if (!cconfig_util_get_str(
            config,
            SDVXIO_KFCA_CONFIG_KFCA_PORT_KEY,
            config_kfca->port,
            sizeof(config_kfca->port) - 1,
            SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_PORT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            SDVXIO_KFCA_CONFIG_KFCA_PORT_KEY,
            SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_PORT_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            SDVXIO_KFCA_CONFIG_KFCA_BAUD_KEY,
            &config_kfca->baud,
            SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_BAUD_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXIO_KFCA_CONFIG_KFCA_BAUD_KEY,
            SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_BAUD_VALUE);
    }
}

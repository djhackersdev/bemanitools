#include "cconfig/cconfig-util.h"

#include "eamio-icca/config-icc.h"

#define EAMIO_ICCA_CONFIG_ICC_PORT_KEY "icc.port"
#define EAMIO_ICCA_CONFIG_ICC_BAUD_KEY "icc.baud"

#define EAMIO_ICCA_CONFIG_ICC_DEFAULT_PORT_VALUE "COM1"
#define EAMIO_ICCA_CONFIG_ICC_DEFAULT_BAUD_VALUE 57600

void eamio_icca_config_icc_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        EAMIO_ICCA_CONFIG_ICC_PORT_KEY,
        EAMIO_ICCA_CONFIG_ICC_DEFAULT_PORT_VALUE,
        "ICCA serial port");

    cconfig_util_set_int(
        config,
        EAMIO_ICCA_CONFIG_ICC_BAUD_KEY,
        EAMIO_ICCA_CONFIG_ICC_DEFAULT_BAUD_VALUE,
        "ICCA bus baudrate (real devices expect 57600)");
}

void eamio_icca_config_icc_get(
    struct icc_config *config_icc, struct cconfig *config)
{
    if (!cconfig_util_get_str(
            config,
            EAMIO_ICCA_CONFIG_ICC_PORT_KEY,
            config_icc->port,
            sizeof(config_icc->port) - 1,
            EAMIO_ICCA_CONFIG_ICC_DEFAULT_PORT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            EAMIO_ICCA_CONFIG_ICC_PORT_KEY,
            EAMIO_ICCA_CONFIG_ICC_DEFAULT_PORT_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            EAMIO_ICCA_CONFIG_ICC_BAUD_KEY,
            &config_icc->baud,
            EAMIO_ICCA_CONFIG_ICC_DEFAULT_BAUD_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            EAMIO_ICCA_CONFIG_ICC_BAUD_KEY,
            EAMIO_ICCA_CONFIG_ICC_DEFAULT_BAUD_VALUE);
    }
}

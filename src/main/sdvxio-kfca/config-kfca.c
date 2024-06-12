#include "cconfig/cconfig-util.h"

#include "iface-core/log.h"

#include "sdvxio-kfca/config-kfca.h"

#define SDVXIO_KFCA_CONFIG_KFCA_PORT_KEY "kfca.port"
#define SDVXIO_KFCA_CONFIG_KFCA_BAUD_KEY "kfca.baud"
#define SDVXIO_KFCA_CONFIG_MAIN_AMP_VOLUME_KEY "kfca.main_override"
#define SDVXIO_KFCA_CONFIG_HEADPHONE_AMP_VOLUME_KEY "kfca.headphone_override"
#define SDVXIO_KFCA_CONFIG_SUBWOOFER_AMP_VOLUME_KEY "kfca.subwoofer_override"

#define SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_PORT_VALUE "COM3"
#define SDVXIO_KFCA_CONFIG_KFCA_DEFAULT_BAUD_VALUE 57600
#define SDVXIO_KFCA_CONFIG_DEFAULT_MAIN_AMP_VOLUME_VALUE -1
#define SDVXIO_KFCA_CONFIG_DEFAULT_HEADPHONE_AMP_VOLUME_VALUE -1
#define SDVXIO_KFCA_CONFIG_DEFAULT_SUBWOOFER_AMP_VOLUME_VALUE -1

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

    cconfig_util_set_int(
        config,
        SDVXIO_KFCA_CONFIG_MAIN_AMP_VOLUME_KEY,
        SDVXIO_KFCA_CONFIG_DEFAULT_MAIN_AMP_VOLUME_VALUE,
        "SDVXIO digital amp main volume (0-96) 0 is high, 96 is low. -1 is no "
        "override.");

    cconfig_util_set_int(
        config,
        SDVXIO_KFCA_CONFIG_HEADPHONE_AMP_VOLUME_KEY,
        SDVXIO_KFCA_CONFIG_DEFAULT_HEADPHONE_AMP_VOLUME_VALUE,
        "SDVXIO digital amp headphone volume (0-96) 0 is high, 96 is low. -1 "
        "is no override.");

    cconfig_util_set_int(
        config,
        SDVXIO_KFCA_CONFIG_SUBWOOFER_AMP_VOLUME_KEY,
        SDVXIO_KFCA_CONFIG_DEFAULT_SUBWOOFER_AMP_VOLUME_VALUE,
        "SDVXIO digital amp subwoofer volume (0-96) 0 is high, 96 is low. -1 "
        "is no override.");
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

    if (!cconfig_util_get_int(
            config,
            SDVXIO_KFCA_CONFIG_MAIN_AMP_VOLUME_KEY,
            &config_kfca->override_main_volume,
            SDVXIO_KFCA_CONFIG_DEFAULT_MAIN_AMP_VOLUME_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXIO_KFCA_CONFIG_MAIN_AMP_VOLUME_KEY,
            SDVXIO_KFCA_CONFIG_DEFAULT_MAIN_AMP_VOLUME_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            SDVXIO_KFCA_CONFIG_HEADPHONE_AMP_VOLUME_KEY,
            &config_kfca->override_headphone_volume,
            SDVXIO_KFCA_CONFIG_DEFAULT_HEADPHONE_AMP_VOLUME_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXIO_KFCA_CONFIG_HEADPHONE_AMP_VOLUME_KEY,
            SDVXIO_KFCA_CONFIG_DEFAULT_HEADPHONE_AMP_VOLUME_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            SDVXIO_KFCA_CONFIG_SUBWOOFER_AMP_VOLUME_KEY,
            &config_kfca->override_sub_volume,
            SDVXIO_KFCA_CONFIG_DEFAULT_SUBWOOFER_AMP_VOLUME_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            SDVXIO_KFCA_CONFIG_SUBWOOFER_AMP_VOLUME_KEY,
            SDVXIO_KFCA_CONFIG_DEFAULT_SUBWOOFER_AMP_VOLUME_VALUE);
    }
}

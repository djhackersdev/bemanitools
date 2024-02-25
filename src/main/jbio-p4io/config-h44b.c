#include "cconfig/cconfig-util.h"

#include "core/log.h"

#include "jbio-p4io/config-h44b.h"

#define JBIO_CONFIG_H44B_PORT_KEY "h44b.port"
#define JBIO_CONFIG_H44B_BAUD_KEY "h44b.baud"

#define JBIO_CONFIG_H44B_DEFAULT_PORT_VALUE "COM2"
#define JBIO_CONFIG_H44B_DEFAULT_BAUD_VALUE 57600

void jbio_config_h44b_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        JBIO_CONFIG_H44B_PORT_KEY,
        JBIO_CONFIG_H44B_DEFAULT_PORT_VALUE,
        "H44B serial port");

    cconfig_util_set_int(
        config,
        JBIO_CONFIG_H44B_BAUD_KEY,
        JBIO_CONFIG_H44B_DEFAULT_BAUD_VALUE,
        "H44B bus baudrate (real devices expect 57600)");
}

void jbio_config_h44b_get(
    struct h44b_config *config_h44b, struct cconfig *config)
{
    if (!cconfig_util_get_str(
            config,
            JBIO_CONFIG_H44B_PORT_KEY,
            config_h44b->port,
            sizeof(config_h44b->port) - 1,
            JBIO_CONFIG_H44B_DEFAULT_PORT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            JBIO_CONFIG_H44B_PORT_KEY,
            JBIO_CONFIG_H44B_DEFAULT_PORT_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            JBIO_CONFIG_H44B_BAUD_KEY,
            &config_h44b->baud,
            JBIO_CONFIG_H44B_DEFAULT_BAUD_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            JBIO_CONFIG_H44B_BAUD_KEY,
            JBIO_CONFIG_H44B_DEFAULT_BAUD_VALUE);
    }
}

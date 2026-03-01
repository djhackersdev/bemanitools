#include "cconfig/cconfig-util.h"

#include "config.h"

#include "util/log.h"

#define DDRIO_CONFIG_P4IO_MDXF_PORT_KEY "mdxf.port"
#define DDRIO_CONFIG_P4IO_MDXF_BAUD_KEY "mdxf.baud"

#define DDRIO_CONFIG_P4IO_MDXF_DEFAULT_PORT_VALUE "COM2"
#define DDRIO_CONFIG_P4IO_MDXF_DEFAULT_BAUD_VALUE 115200

void ddrio_config_p4io_mdxf_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        DDRIO_CONFIG_P4IO_MDXF_PORT_KEY,
        DDRIO_CONFIG_P4IO_MDXF_DEFAULT_PORT_VALUE,
        "MDXF serial port");

    cconfig_util_set_int(
        config,
        DDRIO_CONFIG_P4IO_MDXF_BAUD_KEY,
        DDRIO_CONFIG_P4IO_MDXF_DEFAULT_BAUD_VALUE,
        "MDXF bus baudrate (115200 is high speed, but will respond at 57600)");
}

void ddrio_config_p4io_mdxf_get(
    struct p4io_mdxf_config *config_p4io_mdxf, struct cconfig *config)
{
    if (!cconfig_util_get_str(
            config,
            DDRIO_CONFIG_P4IO_MDXF_PORT_KEY,
            config_p4io_mdxf->port,
            sizeof(config_p4io_mdxf->port) - 1,
            DDRIO_CONFIG_P4IO_MDXF_DEFAULT_PORT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            DDRIO_CONFIG_P4IO_MDXF_PORT_KEY,
            DDRIO_CONFIG_P4IO_MDXF_DEFAULT_PORT_VALUE);
    }

    if (!cconfig_util_get_int(
            config,
            DDRIO_CONFIG_P4IO_MDXF_BAUD_KEY,
            &config_p4io_mdxf->baud,
            DDRIO_CONFIG_P4IO_MDXF_DEFAULT_BAUD_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            DDRIO_CONFIG_P4IO_MDXF_BAUD_KEY,
            DDRIO_CONFIG_P4IO_MDXF_DEFAULT_BAUD_VALUE);
    }
}

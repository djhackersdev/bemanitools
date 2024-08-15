#include "cconfig/cconfig-util.h"

#include "ddrio-p3io/config.h"

#include "iface-core/log.h"

#define DDRIO_P3IO_CONFIG_EXTIO_PORT_KEY "ddrio.p3io.extio_port"

#define DDRIO_P3IO_CONFIG_EXTIO_PORT_VALUE "COM1"

void ddrio_p3io_config_init(struct cconfig *config)
{
    cconfig_util_set_str(
        config,
        DDRIO_P3IO_CONFIG_EXTIO_PORT_KEY,
        DDRIO_P3IO_CONFIG_EXTIO_PORT_VALUE,
        "COM port the EXTIO is connected to, default COM1");
}

void ddrio_p3io_config_get(
    struct ddrio_p3io_config *config_ddrio_p3io, struct cconfig *config)
{
    if (!cconfig_util_get_str(
            config,
            DDRIO_P3IO_CONFIG_EXTIO_PORT_KEY,
            config_ddrio_p3io->extio_port,
            sizeof(config_ddrio_p3io->extio_port) - 1,
            DDRIO_P3IO_CONFIG_EXTIO_PORT_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%s'",
            DDRIO_P3IO_CONFIG_EXTIO_PORT_KEY,
            DDRIO_P3IO_CONFIG_EXTIO_PORT_VALUE);
    }
}

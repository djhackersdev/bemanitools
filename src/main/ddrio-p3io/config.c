#include <string.h>

#include "core/config-ext.h"

#include "ddrio-p3io/config.h"

#include "iface-core/log.h"

static void _ddrio_p3io_config_verify(const ddrio_p3io_config_t *config_ddrio_p3io)
{
    char *num_part;
    int num;

    if (strncmp(config_ddrio_p3io->extio_port, "COM", 3) != 0) {
        log_fatal("Invalid COM port string: %s", config_ddrio_p3io->extio_port);
    }

    num_part = ((char*) config_ddrio_p3io->extio_port) + 3;

    num = atoi(num_part);

    if (num < 1 || num > 99) {
        log_fatal("Invalid COM port number in string: %s", config_ddrio_p3io->extio_port);
    }
}

void ddrio_p3io_config_get(const bt_core_config_t *config, ddrio_p3io_config_t *config_ddrio_p3io)
{
    bt_core_config_ext_str_get(config, "extio_port", config_ddrio_p3io->extio_port, sizeof(config_ddrio_p3io->extio_port));

    _ddrio_p3io_config_verify(config_ddrio_p3io);
}

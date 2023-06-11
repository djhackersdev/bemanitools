#ifndef DDRIO_P3IO_CONFIG_H
#define DDRIO_P3IO_CONFIG_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct ddrio_p3io_config {
    char extio_port[12];
};

void ddrio_p3io_config_init(struct cconfig *config);

void ddrio_p3io_config_get(
    struct ddrio_p3io_config *config_ddrio_p3io, struct cconfig *config);

#endif
#ifndef VIGEM_DDRIO_CONFIG_H
#define VIGEM_DDRIO_CONFIG_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct vigem_ddrio_config {
    bool enable_reactive_light;
};

bool get_vigem_ddrio_config(struct vigem_ddrio_config *config_out);

#endif
#ifndef VIGEM_IIDXIO_CONFIG_H
#define VIGEM_IIDXIO_CONFIG_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct vigem_iidxio_config {
    bool enable_keylight;
    bool relative_analog;
    int32_t cab_light_mode;
    char text_16seg[1024 + 1];
    int32_t text_scroll_cycle_time_ms;
};

bool vigem_iidxio_config_get(struct vigem_iidxio_config *config_out);

#endif
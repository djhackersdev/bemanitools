#ifndef VIGEM_IIDXIO_CONFIG_H
#define VIGEM_IIDXIO_CONFIG_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct vigem_iidxio_config {
    struct tt {
        struct analog {
            bool relative;
            int32_t relative_sensitivity;
        } analog;

        struct button {
            int32_t debounce;
            int32_t threshold;
        } button;

        bool debug_output;
    } tt;
    
    struct cab_light {
        bool enable_keylight;
        int32_t light_mode;
        char text_16seg[1024 + 1];
        int32_t text_scroll_cycle_time_ms;
    } cab_light;
};

bool vigem_iidxio_config_get(struct vigem_iidxio_config *config_out);

#endif
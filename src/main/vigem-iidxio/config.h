#ifndef VIGEM_IIDXIO_CONFIG_H
#define VIGEM_IIDXIO_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include "api/core/config.h"

typedef struct vigem_iidxio_config {
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
} vigem_iidxio_config_t;

void vigem_iidxio_config_get(const bt_core_config_t *config, vigem_iidxio_config_t *config_out);

#endif
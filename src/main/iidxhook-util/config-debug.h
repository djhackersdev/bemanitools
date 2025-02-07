#ifndef IIDXHOOK_CONFIG_DEBUG_H
#define IIDXHOOK_CONFIG_DEBUG_H

#include <windows.h>

#include "cconfig/cconfig.h"

struct iidxhook_config_debug {
    bool enable_frame_perf_graph;
};

void iidxhook_config_debug_init(struct cconfig *config);

void iidxhook_config_debug_get(
    struct iidxhook_config_debug *config_debug, struct cconfig *config);

#endif
#ifndef LAUNCHER_AVS_CONTEXT_H
#define LAUNCHER_AVS_CONTEXT_H

#include <stdint.h>

#include "imports/avs.h"

#include "util/log.h"

#if AVS_VERSION < 1600
#define AVS_HAS_STD_HEAP
#endif

void avs_init(
    struct property *config_prop,
    struct property_node *config_node,
    uint32_t avs_heap_size,
    uint32_t std_heap_size);
void avs_fini(void);

#endif

#include "cconfig/cconfig-util.h"

#include "iidxhook-util/config-debug.h"

#include "util/log.h"

#define IIDXHOOK_CONFIG_DEBUG_ENABLE_FRAME_PERF_GRAPH_KEY "debug.enable_frame_perf_graph"

#define IIDXHOOK_CONFIG_DEBUG_DEFAULT_ENABLE_FRAME_PERF_GRAPH_VALUE false

void iidxhook_config_debug_init(struct cconfig *config)
{
    cconfig_util_set_bool(
        config,
        IIDXHOOK_CONFIG_DEBUG_ENABLE_FRAME_PERF_GRAPH_KEY,
        IIDXHOOK_CONFIG_DEBUG_DEFAULT_ENABLE_FRAME_PERF_GRAPH_VALUE,
        "Enable frame performance graph overlay");
}

void iidxhook_config_debug_get(
    struct iidxhook_config_debug *config_debug, struct cconfig *config)
{
    if (!cconfig_util_get_bool(
            config,
            IIDXHOOK_CONFIG_DEBUG_ENABLE_FRAME_PERF_GRAPH_KEY,
            &config_debug->enable_frame_perf_graph,
            IIDXHOOK_CONFIG_DEBUG_DEFAULT_ENABLE_FRAME_PERF_GRAPH_VALUE)) {
        log_warning(
            "Invalid value for key '%s' specified, fallback "
            "to default '%d'",
            IIDXHOOK_CONFIG_DEBUG_ENABLE_FRAME_PERF_GRAPH_KEY,
            IIDXHOOK_CONFIG_DEBUG_DEFAULT_ENABLE_FRAME_PERF_GRAPH_VALUE);
    }
}

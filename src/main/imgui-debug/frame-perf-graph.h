#ifndef IMGUI_DEBUG_FRAME_PERF_GRAPH_H
#define IMGUI_DEBUG_FRAME_PERF_GRAPH_H

#include "imgui-bt/component.h"

void imgui_debug_frame_perf_graph_init(
    float target_fps,
    imgui_bt_component_t *component);

#endif
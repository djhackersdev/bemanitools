#ifndef IMGUI_BT_COMPONENT_H
#define IMGUI_BT_COMPONENT_H

#include "imgui-bt/cimgui.h"

typedef void (*imgui_bt_component_frame_update_t)(ImGuiContext *ctx);

typedef struct imgui_bt_component {
    imgui_bt_component_frame_update_t frame_update;
} imgui_bt_component_t;

#endif

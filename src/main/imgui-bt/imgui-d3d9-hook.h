#ifndef IMGUI_D3D9_HOOK_H
#define IMGUI_D3D9_HOOK_H

#include "hook/d3d9.h"

#include "imgui-bt/component.h"

void imgui_d3d9_hook_init(const imgui_bt_component_t *components, size_t component_count);
void imgui_d3d9_hook_fini();

HRESULT imgui_hook_d3d9_irp_handler(struct hook_d3d9_irp *irp);

#endif
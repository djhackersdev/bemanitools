#include <windows.h>

#include "hook/d3d9.h"
#include "hook/table.h"

#include "imgui-bt/imgui-d3d9-hook.h"

#include "imgui-debug/frame-perf-graph.h"

#include "util/defs.h"
#include "util/log.h"

static const hook_d3d9_irp_handler_t d3d9_frame_graph_hook_handlers[] = {
    imgui_hook_d3d9_irp_handler
};

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    imgui_bt_component_t debug_frame_graph_component;

    if (reason == DLL_PROCESS_ATTACH) {
        log_to_writer(log_writer_null, NULL);

        imgui_debug_frame_perf_graph_init(60.0f, &debug_frame_graph_component);

        imgui_d3d9_hook_init(&debug_frame_graph_component, 1);
    
        hook_d3d9_init(d3d9_frame_graph_hook_handlers, lengthof(d3d9_frame_graph_hook_handlers));
    }

    return TRUE;
}
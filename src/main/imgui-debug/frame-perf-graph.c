#include <math.h>

#include "imgui-bt/cimgui.h"

#include "imgui-debug/frame-perf-graph.h"
#include "imgui-debug/time-history.h"

#include "util/log.h"

typedef struct imgui_debug_frame_perf_graph {
    float target_time_ms;
    float y_axis_min_time_ms;
    float y_axis_max_time_ms;
    bool show_frame_rate_graph;
} imgui_debug_frame_perf_graph_t;

static const ImVec2 WINDOW_MIN_SIZE = {320, 240};

static imgui_debug_time_history_t _imgui_debug_frame_perf_graph_history;
static imgui_debug_frame_perf_graph_t _imgui_debug_frame_perf_graph;

static void _imgui_debug_frame_perf_frame_rate_graph_draw(
    imgui_debug_frame_perf_graph_t *graph,
    const imgui_debug_time_history_t *history)
{
    float current_value;
    ImVec2 window_size;
    static bool show_labels = true;
    static bool show_target_line = true;
    static bool show_avg_line = true;

    log_assert(history);

    current_value = 1000.0f / imgui_debug_time_history_recent_value_get(history);

    igSetNextWindowSize(WINDOW_MIN_SIZE, ImGuiCond_FirstUseEver);
    igSetNextWindowSizeConstraints(WINDOW_MIN_SIZE, igGetIO()->DisplaySize, NULL, NULL);

    igBegin("Frame Rate Graph", NULL, ImGuiWindowFlags_MenuBar);

    // Add menu bar
    if (igBeginMenuBar()) {
        if (igBeginMenu("Settings", true)) {
            igPushItemWidth(110);

            float min_fps = 1000.0f / graph->y_axis_max_time_ms;
            float max_fps = 1000.0f / graph->y_axis_min_time_ms;
            float target_fps = 1000.0f / graph->target_time_ms;
            
            if (igDragFloat("y-axis min FPS", &min_fps, 1.0f, 1.0f, max_fps - 1.0f, "%.1f", ImGuiSliderFlags_None)) {
                graph->y_axis_max_time_ms = 1000.0f / min_fps;
            }

            if (igDragFloat("y-axis max FPS", &max_fps, 1.0f, min_fps + 1.0f, 1000.0f, "%.1f", ImGuiSliderFlags_None)) {
                graph->y_axis_min_time_ms = 1000.0f / max_fps;
            }

            if (igInputFloat("Target FPS", &target_fps, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (target_fps >= 1.0f && target_fps <= 1000.0f) {
                    graph->target_time_ms = 1000.0f / target_fps;
                } else {
                    target_fps = 1000.0f / graph->target_time_ms;
                }
            }

            igCheckbox("Show reference line labels", &show_labels);
            igCheckbox("Show target reference line", &show_target_line);
            igCheckbox("Show average reference line", &show_avg_line);

            if (igButton("Focus on average", (ImVec2){0, 0})) {
                float avg_fps = 1000.0f / history->avg_time_ms;
                graph->y_axis_min_time_ms = 1000.0f / (avg_fps + 10.0f);
                graph->y_axis_max_time_ms = 1000.0f / fmaxf(avg_fps - 10.0f, 1.0f);
            }

            igSameLine(0, -1);

            if (igButton("Focus on target", (ImVec2){0, 0})) {
                float target_fps = 1000.0f / graph->target_time_ms;
                graph->y_axis_min_time_ms = 1000.0f / (target_fps + 10.0f);
                graph->y_axis_max_time_ms = 1000.0f / fmaxf(target_fps - 10.0f, 1.0f);
            }

            igCheckbox("Show frame rate graph", &graph->show_frame_rate_graph);

            igPopItemWidth();
            igEndMenu();
        }
        igEndMenuBar();
    }

    igGetContentRegionAvail(&window_size);

    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){1, 1, 0, 1});
    igText("Now %.3f FPS", current_value);
    igPopStyleColor(1);
    igSameLine(0, -1);
    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){1, 0, 0, 1});
    igText("Target %.3f FPS", 1000.0f / graph->target_time_ms);
    igPopStyleColor(1);
    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){0, 1, 0, 1});
    igText("Avg %.3f FPS", 1000.0f / history->avg_time_ms);
    igPopStyleColor(1);
    igSameLine(0, -1);
    igText(" Min %.3f FPS  Max %.3f FPS", 1000.0f / history->max_time_ms, 1000.0f / history->min_time_ms);

    // Setup plot area using actual window size, with extra space at top for "FPS" label
    ImVec2 plot_pos;
    igGetCursorScreenPos(&plot_pos);
    plot_pos.y += 15; // Add space at top for "FPS" label
    ImVec2 plot_size = {window_size.x - 50, window_size.y - 65}; // Adjusted for extra top space

    // Convert time values to FPS for plotting
    float fps_values[history->size];
    for(int i = 0; i < history->size; i++) {
        fps_values[i] = 1000.0f / history->time_values_ms[i];
    }

    // Plot FPS values
    ImVec2 plot_pos_offset = {plot_pos.x + 50, plot_pos.y};
    igSetCursorScreenPos(plot_pos_offset);

    igPlotLines_FloatPtr("##framegraph", 
        fps_values, 
        history->size, 
        history->current_index,
        "",
        1000.0f / graph->y_axis_min_time_ms, // Swapped min/max to invert Y axis
        1000.0f / graph->y_axis_max_time_ms,
        plot_size,
        sizeof(float));

    // Draw Y axis (FPS)
    ImDrawList* draw_list = igGetWindowDrawList();
    char y_label[32];
    ImDrawList_AddLine(draw_list, 
        (ImVec2){plot_pos.x + 50, plot_pos.y},
        (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), 1.0f);

    // Draw "FPS" label at top of y-axis
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, plot_pos.y - 15},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), "FPS", NULL);

    // Scale number of reference points based on plot height
    int num_reference_points = (int)(plot_size.y / 25); // One point per ~40 pixels
    if (num_reference_points < 4) num_reference_points = 4;
    
    float fps_min = 1000.0f / graph->y_axis_max_time_ms;
    float fps_max = 1000.0f / graph->y_axis_min_time_ms;
    float fps_step = (fps_max - fps_min) / (num_reference_points + 1);

    // Draw min FPS value at bottom of y-axis and reference line
    snprintf(y_label, sizeof(y_label), "%.3f", fps_min);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, plot_pos.y + plot_size.y - 10}, 
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), y_label, NULL);
    ImDrawList_AddLine(draw_list,
        (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y},
        (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y + plot_size.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,0.3f}), 1.0f);

    // Draw reference points and lines on side of y-axis
    for (int i = 1; i <= num_reference_points; i++) {
        float value = fps_min + (fps_step * i);
        float normalized_pos = 1.0f - ((value - fps_min) / (fps_max - fps_min)); // Inverted normalization
        float y_pos = plot_pos.y + (plot_size.y * normalized_pos);
        snprintf(y_label, sizeof(y_label), "%.3f", value);
        ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, y_pos - 5},
            igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), y_label, NULL);
        ImDrawList_AddLine(draw_list,
            (ImVec2){plot_pos.x + 50, y_pos},
            (ImVec2){plot_pos.x + plot_size.x + 50, y_pos},
            igColorConvertFloat4ToU32((ImVec4){1,1,1,0.2f}), 1.0f);
    }

    // Draw max FPS value at top of y-axis and reference line
    snprintf(y_label, sizeof(y_label), "%.3f", fps_max);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, plot_pos.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), y_label, NULL);
    ImDrawList_AddLine(draw_list,
        (ImVec2){plot_pos.x + 50, plot_pos.y},
        (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,0.3f}), 1.0f);

    // Draw target FPS reference line if within plot area
    float target_fps_value = 1000.0f / graph->target_time_ms;
    if (show_target_line && target_fps_value >= fps_min && target_fps_value <= fps_max) {
        float normalized_target = 1.0f - ((target_fps_value - fps_min) / (fps_max - fps_min)); // Inverted normalization
        float y_pos_target = plot_pos.y + (plot_size.y * normalized_target);
        ImDrawList_AddLine(draw_list,
            (ImVec2){plot_pos.x + 50, y_pos_target},
            (ImVec2){plot_pos.x + plot_size.x + 50, y_pos_target},
            igColorConvertFloat4ToU32((ImVec4){1,0,0,1.0f}), 1.0f);
        if (show_labels) {
            snprintf(y_label, sizeof(y_label), "%.3f", target_fps_value);
            ImDrawList_AddText_Vec2(draw_list, 
                (ImVec2){plot_pos.x + 50 + plot_size.x/2 - 10, y_pos_target + 5},
                igColorConvertFloat4ToU32((ImVec4){1,0,0,1}), y_label, NULL);
        }
    }

    // Draw reference line for current average if within plot area
    float avg_fps = 1000.0f / history->avg_time_ms;
    if (show_avg_line && avg_fps >= fps_min && avg_fps <= fps_max) {
        float normalized_avg = 1.0f - ((avg_fps - fps_min) / (fps_max - fps_min)); // Inverted normalization
        float y_pos_avg = plot_pos.y + (plot_size.y * normalized_avg);
        ImDrawList_AddLine(draw_list,
            (ImVec2){plot_pos.x + 50, y_pos_avg},
            (ImVec2){plot_pos.x + plot_size.x + 50, y_pos_avg},
            igColorConvertFloat4ToU32((ImVec4){0,1,0,1.0f}), 1.0f);
        if (show_labels) {
            snprintf(y_label, sizeof(y_label), "%.3f", avg_fps);
            ImDrawList_AddText_Vec2(draw_list, 
                (ImVec2){plot_pos.x + 50 + plot_size.x/2 - 10, y_pos_avg + 5},
                igColorConvertFloat4ToU32((ImVec4){0,1,0,1}), y_label, NULL);
        }
    }

    // Draw X axis (time in frames)
    ImDrawList_AddLine(draw_list,
        (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y},
        (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y + plot_size.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), 1.0f);

    // Draw "frames" label centered on x-axis
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 50 + (plot_size.x / 2) - 20, plot_pos.y + plot_size.y + 5},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), "frames ago", NULL);

    char x_label[32];
    snprintf(x_label, sizeof(x_label), "%d", history->size);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y + 5},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), x_label, NULL);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y + plot_size.y + 5},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), "0", NULL);

    igEnd();
}

static void _imgui_debug_frame_perf_frame_time_graph_draw(
    imgui_debug_frame_perf_graph_t *graph,
    const imgui_debug_time_history_t *history)
{
    float current_value;
    ImVec2 window_size;
    static bool show_labels = true;
    static bool show_target_line = true;
    static bool show_avg_line = true;

    log_assert(history);

    current_value = imgui_debug_time_history_recent_value_get(history);

    igSetNextWindowSize(WINDOW_MIN_SIZE, ImGuiCond_FirstUseEver);
    igSetNextWindowSizeConstraints(WINDOW_MIN_SIZE, igGetIO()->DisplaySize, NULL, NULL);

    igBegin("Frame Time Graph", NULL, ImGuiWindowFlags_MenuBar);

    // Add menu bar
    if (igBeginMenuBar()) {
        if (igBeginMenu("Settings", true)) {
            igPushItemWidth(110);

            float min_time_slider = graph->y_axis_min_time_ms;
            float max_time_slider = graph->y_axis_max_time_ms;
            float target_time_input = graph->target_time_ms;
            
            if (igDragFloat("y-axis min time (ms)", &min_time_slider, 0.1f, 0.1f, max_time_slider - 0.1f, "%.1f", ImGuiSliderFlags_None)) {
                graph->y_axis_min_time_ms = min_time_slider;
            }

            if (igDragFloat("y-axis max time (ms)", &max_time_slider, 0.1f, min_time_slider + 0.1f, 100.0f, "%.1f", ImGuiSliderFlags_None)) {
                graph->y_axis_max_time_ms = max_time_slider;
            }

            if (igInputFloat("Target time reference (ms)", &target_time_input, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (target_time_input >= 0.1f && target_time_input <= 100.0f) {
                    graph->target_time_ms = target_time_input;
                } else {
                    target_time_input = graph->target_time_ms;
                }
            }

            igCheckbox("Show reference line labels", &show_labels);
            igCheckbox("Show target reference line", &show_target_line);
            igCheckbox("Show average reference line", &show_avg_line);

            if (igButton("Focus on average", (ImVec2){0, 0})) {
                // Convert +/- 10 fps around average to milliseconds
                float avg_fps = 1000.0f / history->avg_time_ms;
                graph->y_axis_min_time_ms = 1000.0f / (avg_fps + 10.0f);
                graph->y_axis_max_time_ms = 1000.0f / fmaxf(avg_fps - 10.0f, 1.0f);
            }

            igSameLine(0, -1);

            if (igButton("Focus on target", (ImVec2){0, 0})) {
                // Convert +/- 10 fps around target to milliseconds
                float target_fps = 1000.0f / graph->target_time_ms;
                graph->y_axis_min_time_ms = 1000.0f / (target_fps + 10.0f);
                graph->y_axis_max_time_ms = 1000.0f / fmaxf(target_fps - 10.0f, 1.0f);
            }

            igCheckbox("Show frame rate graph", &graph->show_frame_rate_graph);

            igPopItemWidth();
            igEndMenu();
        }
        igEndMenuBar();
    }

    igGetContentRegionAvail(&window_size);

    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){1, 1, 0, 1});
    igText("Now %.3f ms", current_value);
    igPopStyleColor(1);
    igSameLine(0, -1);
    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){1, 0, 0, 1});
    igText("Target %.3f ms", graph->target_time_ms);
    igPopStyleColor(1);
    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){0, 1, 0, 1});
    igText("Avg %.3f ms", history->avg_time_ms);
    igPopStyleColor(1);
    igSameLine(0, -1);
    igText(" Min %.3f ms  Max %.3f ms", history->min_time_ms, history->max_time_ms);

    // Setup plot area using actual window size, with extra space at top for "ms" label
    ImVec2 plot_pos;
    igGetCursorScreenPos(&plot_pos);
    plot_pos.y += 15; // Add space at top for "ms" label
    ImVec2 plot_size = {window_size.x - 50, window_size.y - 65}; // Adjusted for extra top space

    // Plot frame times in ms
    ImVec2 plot_pos_offset = {plot_pos.x + 50, plot_pos.y};
    igSetCursorScreenPos(plot_pos_offset);

    igPlotLines_FloatPtr("##framegraph", 
        history->time_values_ms, 
        history->size, 
        history->current_index,
        "",
        graph->y_axis_min_time_ms,
        graph->y_axis_max_time_ms,
        plot_size,
        sizeof(float));

    // Draw Y axis (ms)
    ImDrawList* draw_list = igGetWindowDrawList();
    char y_label[32];
    ImDrawList_AddLine(draw_list, 
        (ImVec2){plot_pos.x + 50, plot_pos.y},
        (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), 1.0f);

    // Draw "ms" label at top of y-axis
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, plot_pos.y - 15},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), "ms", NULL);

    // Scale number of reference points based on plot height
    int num_reference_points = (int)(plot_size.y / 25); // One point per ~40 pixels
    if (num_reference_points < 4) num_reference_points = 4;
    
    float time_min = graph->y_axis_min_time_ms;
    float time_max = graph->y_axis_max_time_ms;
    float time_step = (time_max - time_min) / (num_reference_points + 1);

    // Draw min time value at top of y-axis and reference line
    snprintf(y_label, sizeof(y_label), "%.2f", time_min);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, plot_pos.y + plot_size.y - 10}, 
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), y_label, NULL);
    ImDrawList_AddLine(draw_list,
        (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y},
        (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y + plot_size.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,0.3f}), 1.0f);

    // Draw reference points and lines on side of y-axis
    for (int i = 1; i <= num_reference_points; i++) {
        float value = time_max - (time_step * i);
        float normalized_pos = (time_max - value) / (time_max - time_min);
        float y_pos = plot_pos.y + (plot_size.y * normalized_pos);
        snprintf(y_label, sizeof(y_label), "%.2f", value);
        ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, y_pos - 5},
            igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), y_label, NULL);
        ImDrawList_AddLine(draw_list,
            (ImVec2){plot_pos.x + 50, y_pos},
            (ImVec2){plot_pos.x + plot_size.x + 50, y_pos},
            igColorConvertFloat4ToU32((ImVec4){1,1,1,0.2f}), 1.0f);
    }

    // Draw max time value at bottom of y-axis and reference line
    snprintf(y_label, sizeof(y_label), "%.2f", time_max);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 5, plot_pos.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), y_label, NULL);
    ImDrawList_AddLine(draw_list,
        (ImVec2){plot_pos.x + 50, plot_pos.y},
        (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,0.3f}), 1.0f);

    // Draw target frame time reference line if within plot area
    if (show_target_line && graph->target_time_ms >= time_min && graph->target_time_ms <= time_max) {
        float normalized_target = (time_max - graph->target_time_ms) / (time_max - time_min);
        float y_pos_target = plot_pos.y + (plot_size.y * normalized_target);
        ImDrawList_AddLine(draw_list,
            (ImVec2){plot_pos.x + 50, y_pos_target},
            (ImVec2){plot_pos.x + plot_size.x + 50, y_pos_target},
            igColorConvertFloat4ToU32((ImVec4){1,0,0,1.0f}), 1.0f);
        if (show_labels) {
            snprintf(y_label, sizeof(y_label), "%.3f", graph->target_time_ms);
            ImDrawList_AddText_Vec2(draw_list, 
                (ImVec2){plot_pos.x + 50 + plot_size.x/2 - 10, y_pos_target + 5},
                igColorConvertFloat4ToU32((ImVec4){1,0,0,1}), y_label, NULL);
        }
    }

    // Draw reference line for current average if within plot area
    if (show_avg_line && history->avg_time_ms >= time_min && history->avg_time_ms <= time_max) {
        float normalized_avg = (time_max - history->avg_time_ms) / (time_max - time_min);
        float y_pos_avg = plot_pos.y + (plot_size.y * normalized_avg);
        ImDrawList_AddLine(draw_list,
            (ImVec2){plot_pos.x + 50, y_pos_avg},
            (ImVec2){plot_pos.x + plot_size.x + 50, y_pos_avg},
            igColorConvertFloat4ToU32((ImVec4){0,1,0,1.0f}), 1.0f);
        if (show_labels) {
            snprintf(y_label, sizeof(y_label), "%.3f", history->avg_time_ms);
            ImDrawList_AddText_Vec2(draw_list, 
                (ImVec2){plot_pos.x + 50 + plot_size.x/2 - 10, y_pos_avg + 5},
                igColorConvertFloat4ToU32((ImVec4){0,1,0,1}), y_label, NULL);
        }
    }

    // Draw X axis (time in frames)
    ImDrawList_AddLine(draw_list,
        (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y},
        (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y + plot_size.y},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), 1.0f);

    // Draw "frames" label centered on x-axis
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 50 + (plot_size.x / 2) - 20, plot_pos.y + plot_size.y + 5},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), "frames ago", NULL);

    char x_label[32];
    snprintf(x_label, sizeof(x_label), "%d", history->size);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + 50, plot_pos.y + plot_size.y + 5},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), x_label, NULL);
    ImDrawList_AddText_Vec2(draw_list, (ImVec2){plot_pos.x + plot_size.x + 50, plot_pos.y + plot_size.y + 5},
        igColorConvertFloat4ToU32((ImVec4){1,1,1,1}), "0", NULL);

    igEnd();
}

static void _imgui_debug_frame_perf_graph_component_frame_update(ImGuiContext *ctx)
{
    ImGuiIO *io;

    log_assert(ctx);

    igSetCurrentContext(ctx);
    io = igGetIO();

    imgui_debug_time_history_update(&_imgui_debug_frame_perf_graph_history, 1000.0f / io->Framerate);

    if (_imgui_debug_frame_perf_graph.show_frame_rate_graph) {
        _imgui_debug_frame_perf_frame_rate_graph_draw(&_imgui_debug_frame_perf_graph, &_imgui_debug_frame_perf_graph_history);
    } else {
        _imgui_debug_frame_perf_frame_time_graph_draw(&_imgui_debug_frame_perf_graph, &_imgui_debug_frame_perf_graph_history);
    }
}

void imgui_debug_frame_perf_graph_init(
    float target_fps,
    imgui_bt_component_t *component)
{
    log_assert(target_fps > 0.0f);
    log_assert(component);

    imgui_debug_time_history_init(ceilf(10 * target_fps), &_imgui_debug_frame_perf_graph_history);

    _imgui_debug_frame_perf_graph.target_time_ms = 1000.0f / target_fps;
    _imgui_debug_frame_perf_graph.y_axis_min_time_ms = 1000.0f / fmaxf(0.0f, target_fps - 20.0f);
    _imgui_debug_frame_perf_graph.y_axis_max_time_ms = 1000.0f / fminf(target_fps + 20.0f, 1000.0f);
    _imgui_debug_frame_perf_graph.show_frame_rate_graph = false;

    component->frame_update = _imgui_debug_frame_perf_graph_component_frame_update;
}
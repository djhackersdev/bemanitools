#ifndef IMGUI_DEBUG_TIME_HISTORY_H
#define IMGUI_DEBUG_TIME_HISTORY_H

#include <stdint.h>

typedef struct imgui_debug_time_history {
    uint32_t size;
    float *time_values_ms;
    uint32_t current_index;
    float min_time_ms;
    float max_time_ms;
    float avg_time_ms;
} imgui_debug_time_history_t;

void imgui_debug_time_history_init(uint32_t size, imgui_debug_time_history_t *history);

void imgui_debug_time_history_update(imgui_debug_time_history_t *history, float time_ms);

float imgui_debug_time_history_recent_value_get(const imgui_debug_time_history_t *history);

void imgui_debug_time_history_free(imgui_debug_time_history_t *history);

#endif
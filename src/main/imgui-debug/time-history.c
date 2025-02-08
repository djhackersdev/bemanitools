#include <stdlib.h>
#include <string.h>

#include "time-history.h"

#include "util/log.h"
#include "util/mem.h"

void imgui_debug_time_history_init(uint32_t size, imgui_debug_time_history_t *history)
{
    log_assert(size > 0);
    log_assert(history);

    memset(history, 0, sizeof(imgui_debug_time_history_t));

    history->size = size;
    history->time_values_ms = (float *) xmalloc(size * sizeof(float));
}

void imgui_debug_time_history_update(imgui_debug_time_history_t *history, float time_ms)
{
    log_assert(history);

    history->time_values_ms[history->current_index] = time_ms;
    history->current_index = (history->current_index + 1) % history->size;
    
    history->min_time_ms = history->time_values_ms[0];
    history->max_time_ms = history->time_values_ms[0];
    history->avg_time_ms = 0;

    for (uint32_t i = 0; i < history->size; i++) {
        if (history->time_values_ms[i] < history->min_time_ms) {
            history->min_time_ms = history->time_values_ms[i];
        }

        if (history->time_values_ms[i] > history->max_time_ms) {
            history->max_time_ms = history->time_values_ms[i];
        }

        history->avg_time_ms += history->time_values_ms[i];
    }

    history->avg_time_ms /= history->size;
}

float imgui_debug_time_history_recent_value_get(const imgui_debug_time_history_t *history)
{
    log_assert(history);

    if (history->current_index == 0) {
        return history->time_values_ms[history->size - 1];
    } else {
        return history->time_values_ms[history->current_index - 1];
    }
}

void imgui_debug_time_history_free(imgui_debug_time_history_t *history)
{
    log_assert(history);
    log_assert(history->time_values_ms);
    
    free(history->time_values_ms);
}
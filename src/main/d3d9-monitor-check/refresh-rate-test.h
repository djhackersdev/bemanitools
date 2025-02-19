#ifndef D3D9_MONITOR_CHECK_REFRESH_RATE_TEST_H
#define D3D9_MONITOR_CHECK_REFRESH_RATE_TEST_H

#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"

typedef struct refresh_rate_test_results_t {
    uint32_t total_warm_up_frame_count;
    uint32_t total_sample_frame_count;
    double avg_frame_time_ms;
    double avg_refresh_rate_hz;
} refresh_rate_test_results_t;

typedef struct refresh_rate_test refresh_rate_test_t;

bool refresh_rate_test_init(
    gfx_t *gfx,
    uint32_t total_warm_up_frame_count,
    uint32_t total_sample_frame_count,
    refresh_rate_test_t **test);

bool refresh_rate_test_frame_update(refresh_rate_test_t *test);

void refresh_rate_test_results_get(const refresh_rate_test_t *test, refresh_rate_test_results_t *results);

bool refresh_rate_test_results_frame_update(refresh_rate_test_t *test, uint32_t results_timeout_seconds);

void refresh_rate_test_fini(refresh_rate_test_t *test);

#endif

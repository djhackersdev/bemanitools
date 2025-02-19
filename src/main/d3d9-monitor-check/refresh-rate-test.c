#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "d3d9-monitor-check/gfx.h"
#include "d3d9-monitor-check/font.h"
#include "d3d9-monitor-check/refresh-rate-test.h"

#include "util/mem.h"

typedef enum refresh_rate_test_phase {
    REFRESH_RATE_TEST_PHASE_WARM_UP = 0,
    REFRESH_RATE_TEST_PHASE_MEASURE = 1,
    REFRESH_RATE_TEST_PHASE_DONE = 2
} refresh_rate_test_phase_t;

typedef struct refresh_rate_test_ctx {
    gfx_t *gfx;
    font_t *font;
    uint32_t total_warm_up_frame_count;
    uint32_t total_sample_frame_count;
} refresh_rate_test_ctx_t;

typedef struct refresh_rate_test_state {
    gfx_info_t gfx_info;
    refresh_rate_test_phase_t phase;
    uint64_t last_frame_time_us;
    uint32_t warm_up_frame_count;
    uint32_t sample_frame_count;
    uint64_t warm_up_elapsed_us;
    uint64_t sample_elapsed_us;
} refresh_rate_test_state_t;

typedef struct refresh_rate_test {
    refresh_rate_test_ctx_t ctx;
    refresh_rate_test_state_t state;
} refresh_rate_test_t;

static refresh_rate_test_phase_t _refresh_rate_test_phase_evaluate(const refresh_rate_test_t *test)
{
    switch (test->state.phase) {
        case REFRESH_RATE_TEST_PHASE_WARM_UP:
            if (test->state.warm_up_frame_count < test->ctx.total_warm_up_frame_count) {
                return REFRESH_RATE_TEST_PHASE_WARM_UP;
            } else {
                return REFRESH_RATE_TEST_PHASE_MEASURE;
            }

        case REFRESH_RATE_TEST_PHASE_MEASURE:
            if (test->state.sample_frame_count < test->ctx.total_sample_frame_count) {
                return REFRESH_RATE_TEST_PHASE_MEASURE;
            } else {
                return REFRESH_RATE_TEST_PHASE_DONE;
            }

        case REFRESH_RATE_TEST_PHASE_DONE:
            return REFRESH_RATE_TEST_PHASE_DONE;

        default:
            assert(0);
            return REFRESH_RATE_TEST_PHASE_DONE;
    }
}

bool refresh_rate_test_init(
    gfx_t *gfx,
    uint32_t total_warm_up_frame_count,
    uint32_t total_sample_frame_count,
    refresh_rate_test_t **test)
{
    assert(gfx);
    assert(test);

    *test = xmalloc(sizeof(refresh_rate_test_t));
    memset(*test, 0, sizeof(refresh_rate_test_t));

    if (!font_init(gfx, 20, &(*test)->ctx.font)) {
        free(*test);
        return false;
    }

    font_init(gfx, 20, &(*test)->ctx.font);

    (*test)->ctx.gfx = gfx;
    (*test)->ctx.total_warm_up_frame_count = total_warm_up_frame_count;
    (*test)->ctx.total_sample_frame_count = total_sample_frame_count;

    (*test)->state.phase = REFRESH_RATE_TEST_PHASE_WARM_UP;
    (*test)->state.last_frame_time_us = 0;
    (*test)->state.warm_up_frame_count = 0;
    (*test)->state.sample_frame_count = 0;
    (*test)->state.warm_up_elapsed_us = 0;
    (*test)->state.sample_elapsed_us = 0;
    
    gfx_info_get(gfx, &(*test)->state.gfx_info);

    return true;
}

static bool _refresh_rate_test_phase_warm_up_frame_update(refresh_rate_test_t *test)
{
    font_text_t text;

    assert(test);

    if (!gfx_frame_begin(test->ctx.gfx)) {
        return false;
    }

    font_text_begin(test->ctx.font, 0, 10, 10, &text);

    font_text_white_draw(&text, "Refresh Rate Test");
    font_text_newline(&text);
    font_text_white_draw(&text, "GPU: %s", test->state.gfx_info.adapter_identifier);
    font_text_newline(&text);
    font_text_white_draw(&text, "Spec: %d x %d @ %d hz, %s, vsync %s", test->state.gfx_info.width, test->state.gfx_info.height, test->state.gfx_info.refresh_rate, 
        test->state.gfx_info.windowed ? "windowed" : "fullscreen", test->state.gfx_info.vsync ? "on" : "off");
    font_text_newline(&text);
    font_text_newline(&text);

    // First frame won't have any data available causing division by zero in the stats
    if (test->state.warm_up_frame_count != 0) {
        font_text_white_draw(&text, "Status: Warm-up in progress ...");
        font_text_newline(&text);

        font_text_white_draw(&text, "Frame: %d / %d", test->state.warm_up_frame_count, test->ctx.total_warm_up_frame_count);
        font_text_newline(&text);
        font_text_white_draw(&text, "Last frame time: %.3f ms", test->state.last_frame_time_us / 1000.0f);
        font_text_newline(&text);
        font_text_white_draw(&text, "Avg frame time: %.3f ms", test->state.warm_up_elapsed_us / test->state.warm_up_frame_count / 1000.0f);
        font_text_newline(&text);
        font_text_white_draw(&text, "Last refresh rate: %.3f Hz", 1000.0f / (test->state.last_frame_time_us / 1000.0f));
        font_text_newline(&text);
        font_text_white_draw(&text, "Avg refresh rate: %.3f Hz", 1000.0f / (test->state.warm_up_elapsed_us / test->state.warm_up_frame_count / 1000.0f));
        font_text_newline(&text);
    }

    font_text_newline(&text);
    font_text_white_draw(&text, "Press ESC to exit early");

    font_text_end(&text);

    gfx_frame_end(test->ctx.gfx);

    test->state.last_frame_time_us = gfx_last_frame_time_us_get(test->ctx.gfx);
    test->state.warm_up_elapsed_us += test->state.last_frame_time_us;
    test->state.warm_up_frame_count++;

    return true;
}

static bool _refresh_rate_test_phase_measure_frame_update(refresh_rate_test_t *test)
{
    font_text_t text;

    assert(test);

    if (!gfx_frame_begin(test->ctx.gfx)) {
        return false;
    }

    font_text_begin(test->ctx.font, 0, 10, 10, &text);

    font_text_white_draw(&text, "Refresh Rate Test");
    font_text_newline(&text);
    font_text_white_draw(&text, "GPU: %s", test->state.gfx_info.adapter_identifier);
    font_text_newline(&text);
    font_text_white_draw(&text, "Spec: %d x %d @ %d hz, %s, vsync %s", test->state.gfx_info.width, test->state.gfx_info.height, test->state.gfx_info.refresh_rate, 
        test->state.gfx_info.windowed ? "windowed" : "fullscreen", test->state.gfx_info.vsync ? "on" : "off");
    font_text_newline(&text);
    font_text_newline(&text);

    // First frame won't have any data available causing division by zero in the stats
    if (test->state.sample_frame_count != 0) {
        font_text_white_draw(&text, "Status: Measuring in progress ...");
        font_text_newline(&text);

        font_text_white_draw(&text, "Frame: %d / %d", test->state.sample_frame_count, test->ctx.total_sample_frame_count);
        font_text_newline(&text);
        font_text_white_draw(&text, "Last frame time: %.3f ms", test->state.last_frame_time_us / 1000.0f);
        font_text_newline(&text);
        font_text_white_draw(&text, "Avg frame time: %.3f ms", test->state.sample_elapsed_us / test->state.sample_frame_count / 1000.0f);
        font_text_newline(&text);
        font_text_white_draw(&text, "Last refresh rate: %.3f Hz", 1000.0f / (test->state.last_frame_time_us / 1000.0f));
        font_text_newline(&text);
        font_text_white_draw(&text, "Avg refresh rate: %.3f Hz", 1000.0f / (test->state.sample_elapsed_us / test->state.sample_frame_count / 1000.0f));
        font_text_newline(&text);
    }

    font_text_newline(&text);
    font_text_white_draw(&text, "Press ESC to exit early");

    font_text_end(&text);

    gfx_frame_end(test->ctx.gfx);

    test->state.last_frame_time_us = gfx_last_frame_time_us_get(test->ctx.gfx);
    test->state.sample_elapsed_us += test->state.last_frame_time_us;
    test->state.sample_frame_count++;

    return true;
}

bool refresh_rate_test_frame_update(refresh_rate_test_t *test)
{
    assert(test);

    test->state.phase = _refresh_rate_test_phase_evaluate(test);

    switch (test->state.phase) {
        case REFRESH_RATE_TEST_PHASE_WARM_UP:
            return _refresh_rate_test_phase_warm_up_frame_update(test);

        case REFRESH_RATE_TEST_PHASE_MEASURE:
            return _refresh_rate_test_phase_measure_frame_update(test);

        case REFRESH_RATE_TEST_PHASE_DONE:
            return false;

        default:
            assert(0);
            return false;
    }
}

void refresh_rate_test_results_get(const refresh_rate_test_t *test, refresh_rate_test_results_t *results)
{
    assert(test);
    assert(results);

    results->total_warm_up_frame_count = test->state.warm_up_frame_count;
    results->total_sample_frame_count = test->state.sample_frame_count;

    if (test->state.sample_elapsed_us > 0 && test->state.sample_frame_count > 0) {
        results->avg_frame_time_ms = test->state.sample_elapsed_us / test->state.sample_frame_count / 1000.0f;
        results->avg_refresh_rate_hz = 1000.0f / results->avg_frame_time_ms;
    } else {
        results->avg_frame_time_ms = 0;
        results->avg_refresh_rate_hz = 0;
    }
}

bool refresh_rate_test_results_frame_update(refresh_rate_test_t *test, uint32_t results_timeout_seconds)
{
    refresh_rate_test_results_t results;
    font_text_t text;

    assert(test);

    if (!gfx_frame_begin(test->ctx.gfx)) {
        return false;
    }

    refresh_rate_test_results_get(test, &results);

    font_text_begin(test->ctx.font, 0, 10, 10, &text);

    font_text_white_draw(&text, "Refresh Rate Test");
    font_text_newline(&text);
    font_text_white_draw(&text, "GPU: %s", test->state.gfx_info.adapter_identifier);
    font_text_newline(&text);
    font_text_white_draw(&text, "Spec: %d x %d @ %d hz, %s, vsync %s", test->state.gfx_info.width, test->state.gfx_info.height, test->state.gfx_info.refresh_rate, 
        test->state.gfx_info.windowed ? "windowed" : "fullscreen", test->state.gfx_info.vsync ? "on" : "off");
    font_text_newline(&text);
    font_text_newline(&text);

    if (test->ctx.total_warm_up_frame_count != test->state.warm_up_frame_count || 
            test->ctx.total_sample_frame_count != test->state.sample_frame_count) {
        font_text_white_draw(&text, "Status: Warning, exited early");
    } else {
        font_text_white_draw(&text, "Status: Completed");
    }

    font_text_newline(&text);
    font_text_white_draw(&text, "Total warm-up frame count: %d", results.total_warm_up_frame_count);
    font_text_newline(&text);
    font_text_white_draw(&text, "Total sample frame count: %d", results.total_sample_frame_count);
    font_text_newline(&text);
    font_text_white_draw(&text, "Avg frame time: %.3f ms", results.avg_frame_time_ms);
    font_text_newline(&text);
    font_text_white_draw(&text, "Avg refresh rate: %.3f Hz", results.avg_refresh_rate_hz);
    font_text_newline(&text);
    font_text_newline(&text);

    font_text_white_draw(&text, "Exiting in %d seconds ...", results_timeout_seconds);
    font_text_newline(&text);
    font_text_white_draw(&text, "Press ESC to exit immediately");

    font_text_end(&text);

    gfx_frame_end(test->ctx.gfx);

    return true;
}

void refresh_rate_test_fini(refresh_rate_test_t *test)
{
    assert(test);

    font_fini(test->ctx.font);

    free(test);
}
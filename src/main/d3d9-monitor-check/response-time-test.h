#ifndef D3D9_MONITOR_CHECK_RESPONSE_TIME_TEST_H
#define D3D9_MONITOR_CHECK_RESPONSE_TIME_TEST_H

#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"

typedef struct response_time_test response_time_test_t;

bool response_time_test_init(
    gfx_t *gfx,
    response_time_test_t **test);

bool response_time_test_frame_update(response_time_test_t *test);

void response_time_test_fini(response_time_test_t *test);

#endif // D3D9_MONITOR_CHECK_RESPONSE_TIME_TEST_H

#ifndef D3D9_MONITOR_CHECK_VSYNC_TEST_H
#define D3D9_MONITOR_CHECK_VSYNC_TEST_H

#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"

typedef struct vsync_test vsync_test_t;

bool vsync_test_init(
    gfx_t *gfx,
    vsync_test_t **test);

bool vsync_test_frame_update(vsync_test_t *test);

void vsync_test_fini(vsync_test_t *test);

#endif // D3D9_MONITOR_CHECK_VSYNC_TEST_H

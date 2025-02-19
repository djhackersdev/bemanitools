#include <windows.h>

#include <d3d9.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"

#include "util/mem.h"
#include "util/time.h"

#define DEFAULT_SCROLL_SPEED_PX_PER_SEC 200.0f
#define DEFAULT_BLOCK_DISTANCE_PX 50
#define RECT_WIDTH_PX 80
#define RECT_HEIGHT_PX 20

typedef struct response_time_test {
    gfx_t *gfx;
    float scroll_speed_px_per_sec;
    uint32_t block_distance_px;
    float current_y_pos;
} response_time_test_t;

bool response_time_test_init(
    gfx_t *gfx,
    response_time_test_t **test)
{
    assert(gfx);
    assert(test);

    *test = xmalloc(sizeof(response_time_test_t));

    (*test)->gfx = gfx;
    (*test)->scroll_speed_px_per_sec = DEFAULT_SCROLL_SPEED_PX_PER_SEC;
    (*test)->block_distance_px = DEFAULT_BLOCK_DISTANCE_PX;
    (*test)->current_y_pos = 0;

    return true;
}

bool response_time_test_frame_update(response_time_test_t *test)
{
    IDirect3DDevice9 *device;
    uint32_t screen_height;
    uint32_t screen_width;
    float delta_time;
    D3DRECT rect_white;
    D3DRECT rect_blue;
    
    assert(test);

    device = gfx_device_get(test->gfx);
    screen_width = gfx_width_get(test->gfx);
    screen_height = gfx_height_get(test->gfx);

    delta_time = gfx_last_frame_time_us_get(test->gfx) / 1000000.0f;

    // Update position
    test->current_y_pos += test->scroll_speed_px_per_sec * delta_time;

    // Loop position when rectangles are off screen
    if (test->current_y_pos > screen_height + RECT_HEIGHT_PX) {
        test->current_y_pos = -RECT_HEIGHT_PX;
    }

    // Calculate rectangle positions
    rect_white.x1 = (screen_width / 2) - RECT_WIDTH_PX - 10;
    rect_white.x2 = rect_white.x1 + RECT_WIDTH_PX;
    rect_white.y1 = (int) test->current_y_pos;
    rect_white.y2 = rect_white.y1 + RECT_HEIGHT_PX;

    rect_blue.x1 = (screen_width / 2) + 10;
    rect_blue.x2 = rect_blue.x1 + RECT_WIDTH_PX;
    rect_blue.y1 = (int) test->current_y_pos + test->block_distance_px;
    rect_blue.y2 = rect_blue.y1 + RECT_HEIGHT_PX;

    if (!gfx_frame_begin(test->gfx)) {
        return false;
    }

    // Background
    IDirect3DDevice9_Clear(
        device,
        0,
        NULL,
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(128, 128, 128),
        1.0f,
        0);

    // White rectangle
    IDirect3DDevice9_Clear(
        device,
        1,
        &rect_white,
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(255, 255, 255),
        1.0f,
        0);

    // Blue rectangle
    IDirect3DDevice9_Clear(
        device,
        1,
        &rect_blue,
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(0, 0, 255),
        1.0f,
        0);

    gfx_frame_end(test->gfx);

    return true;
}

void response_time_test_fini(response_time_test_t *test)
{
    assert(test);

    free(test);
}

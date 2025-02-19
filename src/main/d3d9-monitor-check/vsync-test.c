#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"
#include "d3d9-monitor-check/font.h"

#include "util/mem.h"

typedef struct vsync_test {
    gfx_t *gfx;
    font_t *font;
} vsync_test_t;

bool vsync_test_init(
    gfx_t *gfx,
    vsync_test_t **test)
{
    assert(gfx);
    assert(test);

    *test = xmalloc(sizeof(vsync_test_t));
    memset(*test, 0, sizeof(vsync_test_t));

    if (!font_init(gfx, 160, &(*test)->font)) {
        free(*test);
        return false;
    }

    (*test)->gfx = gfx;

    return true;
}

bool vsync_test_frame_update(vsync_test_t *test)
{
    uint64_t frame_count;
    font_text_t text;

    assert(test);

    frame_count = gfx_last_frame_count_get(test->gfx);

    if (!gfx_frame_begin(test->gfx)) {
        return false;
    }

    font_text_begin(test->font, 0, 80, 80, &text);

    if (frame_count % 2 == 0) {
        font_text_red_draw(&text, "VSYNC");
    } else {
        font_text_cyan_draw(&text, "VSYNC");
    }

    font_text_end(&text);

    gfx_frame_end(test->gfx);

    return true;
}

void vsync_test_fini(vsync_test_t *test)
{
    assert(test);

    font_fini(test->font);
    free(test);
}
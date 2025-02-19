#include <assert.h>

#include "d3d9-monitor-check/gfx.h"
#include "d3d9-monitor-check/font.h"
#include "d3d9-monitor-check/menu.h"

#include "util/mem.h"

typedef struct menu {
    gfx_t *gfx;
    font_t *font;
    menu_item_t current_selected_item;
} menu_t;

bool menu_init(
    gfx_t *gfx,
    menu_t **menu)
{
    assert(gfx);
    assert(menu);

    *menu = xmalloc(sizeof(menu_t));
    (*menu)->gfx = gfx;
    (*menu)->current_selected_item = MENU_ITEM_REFRESH_RATE_TEST;

    if (!font_init(gfx, 20, &(*menu)->font)) {
        free(*menu);
        return false;
    }

    return true;
}

void menu_select_cursor_move_up(menu_t *menu)
{
    assert(menu);

    if (menu->current_selected_item == MENU_ITEM_REFRESH_RATE_TEST) {
        menu->current_selected_item = MENU_ITEM_EXIT;
    } else {
        menu->current_selected_item--;
    }
}

void menu_select_cursor_move_down(menu_t *menu)
{
    assert(menu);

    if (menu->current_selected_item == MENU_ITEM_EXIT) {
        menu->current_selected_item = MENU_ITEM_REFRESH_RATE_TEST;
    } else {
        menu->current_selected_item++;
    }
}

menu_item_t menu_item_selected_get(menu_t *menu)
{
    assert(menu);

    return menu->current_selected_item;
}

bool menu_frame_update(menu_t *menu)
{
    font_text_t text;

    assert(menu);

    if (!gfx_frame_begin(menu->gfx)) {
        return false;
    }

    font_text_begin(menu->font, 10, 10, 10, &text);
    font_text_white_draw(&text, "D3D9 Monitor Check - Main Menu");
    font_text_newline(&text);
    font_text_newline(&text);
    if (menu->current_selected_item == MENU_ITEM_REFRESH_RATE_TEST) {
        font_text_white_draw(&text, "-> 1. Refresh Rate Test");
    } else {
        font_text_white_draw(&text, "   1. Refresh Rate Test");
    }
    font_text_newline(&text);
    if (menu->current_selected_item == MENU_ITEM_RESPONSE_TIME_TEST) {
        font_text_white_draw(&text, "-> 2. Response Time Test");
    } else {
        font_text_white_draw(&text, "   2. Response Time Test");
    }
    font_text_newline(&text);
    if (menu->current_selected_item == MENU_ITEM_VSYNC_TEST) {
        font_text_white_draw(&text, "-> 3. VSync Test");
    } else {
        font_text_white_draw(&text, "   3. VSync Test");
    }
    font_text_newline(&text);
    if (menu->current_selected_item == MENU_ITEM_EXIT) {
        font_text_white_draw(&text, "-> 4. Exit");
    } else {
        font_text_white_draw(&text, "   4. Exit");
    }
    font_text_end(&text);

    gfx_frame_end(menu->gfx);

    return true;
}

void menu_fini(menu_t *menu)
{
    assert(menu);

    font_fini(menu->font);

    free(menu);
}
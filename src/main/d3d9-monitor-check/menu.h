#ifndef D3D9_MONITOR_CHECK_MENU_H
#define D3D9_MONITOR_CHECK_MENU_H

#include <stdbool.h>
#include <stdint.h>

#include "d3d9-monitor-check/gfx.h"

typedef enum MENU_ITEM {
    MENU_ITEM_REFRESH_RATE_TEST = 0,
    MENU_ITEM_RESPONSE_TIME_TEST = 1,
    MENU_ITEM_VSYNC_TEST = 2,
    MENU_ITEM_EXIT = 3,
} menu_item_t;

typedef struct menu menu_t;

bool menu_init(
    gfx_t *gfx,
    menu_t **menu);

void menu_select_cursor_move_up(menu_t *menu);

void menu_select_cursor_move_down(menu_t *menu);

menu_item_t menu_item_selected_get(menu_t *menu);

bool menu_frame_update(menu_t *menu);

void menu_fini(menu_t *menu);

#endif
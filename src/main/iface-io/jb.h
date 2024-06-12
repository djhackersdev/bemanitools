#ifndef API_IO_JB_H
#define API_IO_JB_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/jb.h"

void bt_io_jb_api_set(const bt_io_jb_api_t *api);
void bt_io_jb_api_get(bt_io_jb_api_t *api);
void bt_io_jb_api_clear();

bool bt_io_jb_init();
void bt_io_jb_fini();
bool bt_io_jb_inputs_read();
uint8_t bt_io_jb_sys_inputs_get();
uint16_t bt_io_jb_panel_inputs_get();
void bt_io_jb_rgb_led_set(
    bt_io_jb_rgb_led_t unit, uint8_t r, uint8_t g, uint8_t b);
bool bt_io_jb_lights_write();
bool bt_io_jb_panel_mode_set(bt_io_jb_panel_mode_t mode);
bool bt_io_jb_coin_blocker_set(bool blocked);

#endif
#ifndef API_IO_IIDX_H
#define API_IO_IIDX_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/iidx.h"

void bt_io_iidx_api_set(const bt_io_iidx_api_t *api);
void bt_io_iidx_api_get(bt_io_iidx_api_t *api);
void bt_io_iidx_api_clear();

bool bt_io_iidx_init();
void bt_io_iidx_fini();
void bt_io_iidx_ep1_deck_lights_set(uint16_t deck_lights);
void bt_io_iidx_ep1_panel_lights_set(uint8_t panel_lights);
void bt_io_iidx_ep1_top_lamps_set(uint8_t top_lamps);
void bt_io_iidx_ep1_top_neons_set(bool top_neons);
bool bt_io_iidx_ep1_send();
bool bt_io_iidx_ep2_recv();
uint8_t bt_io_iidx_ep2_turntable_get(uint8_t player_no);
uint8_t bt_io_iidx_ep2_slider_get(uint8_t slider_no);
uint8_t bt_io_iidx_ep2_sys_get();
uint8_t bt_io_iidx_ep2_panel_get();
uint16_t bt_io_iidx_ep2_keys_get();
bool bt_io_iidx_ep3_16seg_send(const char *text);

#endif
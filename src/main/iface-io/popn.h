#ifndef API_IO_POPN_H
#define API_IO_POPN_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/popn.h"

void bt_io_popn_api_set(const bt_io_popn_api_t *api);
void bt_io_popn_api_get(bt_io_popn_api_t *api);
void bt_io_popn_api_clear();

bool bt_io_popn_init();
void bt_io_popn_fini();

uint32_t bt_io_popn_buttons_get();
void bt_io_popn_top_lights_set(uint32_t lights);
void bt_io_popn_side_lights_set(uint32_t lights);
void bt_io_popn_button_lights_set(uint32_t lights);
void bt_io_popn_coin_blocker_light_set(bool enabled);
void bt_io_popn_coin_counter_light_set(bool enabled);

#endif
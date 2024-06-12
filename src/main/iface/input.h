#ifndef BT_INPUT_H
#define BT_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#include "api/input.h"

void bt_input_api_set(const bt_input_api_t *api);
void bt_input_api_get(bt_input_api_t *api);
void bt_input_api_clear();

bool bt_input_init();
void bt_input_fini();

bool bt_input_mapper_config_load(const char *game_type);
uint8_t bt_input_mapper_analog_read(uint8_t analog);
uint64_t bt_input_mapper_update();
void bt_input_mapper_light_write(uint8_t light, uint8_t intensity);

#endif
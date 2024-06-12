#ifndef API_IO_EAM_H
#define API_IO_EAM_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/eam.h"

void bt_io_eam_api_set(const bt_io_eam_api_t *api);
void bt_io_eam_api_get(bt_io_eam_api_t *api);
void bt_io_eam_api_clear();

bool bt_io_eam_init();
void bt_io_eam_fini();

uint16_t bt_io_eam_keypad_state_get(uint8_t unit_no);
uint8_t bt_io_eam_sensor_state_get(uint8_t unit_no);
uint8_t bt_io_eam_card_read(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes);
bool bt_io_eam_card_slot_cmd_send(uint8_t unit_no, uint8_t cmd);
bool bt_io_eam_poll(uint8_t unit_no);
const bt_io_eam_config_api_t *bt_io_eam_config_api_get();

#endif
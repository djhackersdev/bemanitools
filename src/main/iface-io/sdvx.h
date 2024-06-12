#ifndef API_IO_SDVX_H
#define API_IO_SDVX_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/sdvx.h"

void bt_io_sdvx_api_set(const bt_io_sdvx_api_t *api);
void bt_io_sdvx_api_get(bt_io_sdvx_api_t *api);
void bt_io_sdvx_api_clear();

bool bt_io_sdvx_init();
void bt_io_sdvx_fini();

void bt_io_sdvx_gpio_lights_set(uint32_t gpio_lights);
void bt_io_sdvx_pwm_light_set(uint8_t light_no, uint8_t intensity);
bool bt_io_sdvx_output_write();
bool bt_io_sdvx_input_read();
uint8_t bt_io_sdvx_input_gpio_sys_get();
uint16_t bt_io_sdvx_input_gpio_get(uint8_t gpio_bank);
uint16_t bt_io_sdvx_spinner_pos_get(uint8_t spinner_no);
bool bt_io_sdvx_amp_volume_set(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer);

#endif
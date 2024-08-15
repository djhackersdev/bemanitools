#ifndef API_IO_DDR_H
#define API_IO_DDR_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/ddr.h"

void bt_io_ddr_api_set(const bt_io_ddr_api_t *api);
void bt_io_ddr_api_get(bt_io_ddr_api_t *api);
void bt_io_ddr_api_clear();

bool bt_io_ddr_init();
void bt_io_ddr_fini();

uint32_t bt_io_ddr_pad_read();
void bt_io_ddr_extio_lights_set(uint32_t extio_lights);
void bt_io_ddr_p3io_lights_set(uint32_t p3io_lights);
void bt_io_ddr_hdxs_lights_panel_set(uint32_t hdxs_lights);
void bt_io_ddr_hdxs_lights_rgb_set(
    uint8_t idx, uint8_t r, uint8_t g, uint8_t b);

#endif
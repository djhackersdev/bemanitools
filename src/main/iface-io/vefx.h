#ifndef API_IO_VEFX_H
#define API_IO_VEFX_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/vefx.h"

void bt_io_vefx_api_set(const bt_io_vefx_api_t *api);
void bt_io_vefx_api_get(bt_io_vefx_api_t *api);
void bt_io_vefx_api_clear();

bool bt_io_vefx_init();
void bt_io_vefx_fini();

bool bt_io_vefx_recv(uint64_t *ppad);
uint8_t bt_io_vefx_slider_get(uint8_t slider_no);
bool bt_io_vefx_16seg_send(const char *text);

#endif
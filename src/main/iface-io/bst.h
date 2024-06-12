#ifndef API_IO_BST_H
#define API_IO_BST_H

#include <stdbool.h>
#include <stdint.h>

#include "api/io/bst.h"

void bt_io_bst_api_set(const bt_io_bst_api_t *api);
void bt_io_bst_api_get(bt_io_bst_api_t *api);
void bt_io_bst_api_clear();

bool bt_io_bst_init();
void bt_io_bst_fini();

bool bt_io_bst_input_read();
uint8_t bt_io_bst_input_get();

#endif
#ifndef BTAPI_IO_BST_H
#define BTAPI_IO_BST_H

#include <stdbool.h>
#include <stdint.h>

typedef enum bt_io_bst_gpio_sys_bit {
    BT_IO_BST_GPIO_SYS_COIN = 2,
    BT_IO_BST_GPIO_SYS_TEST = 4,
    BT_IO_BST_GPIO_SYS_SERVICE = 5,
} bt_io_bst_gpio_sys_bit_t;

typedef bool (*bt_io_bst_init_t)();
typedef void (*bt_io_bst_fini_t)();
typedef bool (*bt_io_bst_input_read_t)();
typedef uint8_t (*bt_io_bst_input_get_t)();

bool bt_io_bst_init();
void bt_io_bst_fini();
bool bt_io_bst_input_read();
uint8_t bt_io_bst_input_get();

#endif
#ifndef BT_IO_BST_H
#define BT_IO_BST_H

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

typedef struct bt_io_bst_impl {
    bt_io_bst_init_t init;
    bt_io_bst_fini_t fini;
    bt_io_bst_input_read_t input_read;
    bt_io_bst_input_get_t input_get;
} bt_io_bst_impl_t;

void bt_io_bst_impl_init(bt_io_bst_impl_t *impl);

#endif
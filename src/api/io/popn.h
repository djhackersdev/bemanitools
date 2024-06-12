#ifndef BT_API_IO_POPN_H
#define BT_API_IO_POPN_H

/* IO emulation provider for pop'n music. */

#include <stdbool.h>
#include <stdint.h>

/* Bit mapping for the "buttons" word */

typedef enum bt_io_popn_sys_bit {
    BT_IO_POPN_SYS_SERVICE = 0x06,
    BT_IO_POPN_SYS_TEST = 0x07,
} bt_io_popn_sys_bit_t;

typedef enum bt_io_popn_button_bit {
    BT_IO_POPN_BUTTON_1 = 0x08,
    BT_IO_POPN_BUTTON_2 = 0x09,
    BT_IO_POPN_BUTTON_3 = 0x0a,
    BT_IO_POPN_BUTTON_4 = 0x0b,
    BT_IO_POPN_BUTTON_5 = 0x0c,
    BT_IO_POPN_BUTTON_6 = 0x0d,
    BT_IO_POPN_BUTTON_7 = 0x0e,
    BT_IO_POPN_BUTTON_8 = 0x0f,
    BT_IO_POPN_BUTTON_9 = 0x10,
} bt_io_popn_button_bit_t;

typedef enum bt_io_popn_light_top_lamp_bit {
    BT_IO_POPN_LIGHT_HI_LAMP1 = 0x00,
    BT_IO_POPN_LIGHT_HI_LAMP2 = 0x01,
    BT_IO_POPN_LIGHT_HI_LAMP3 = 0x02,
    BT_IO_POPN_LIGHT_HI_LAMP4 = 0x03,
    BT_IO_POPN_LIGHT_HI_LAMP5 = 0x04,
} bt_io_popn_light_top_lamp_bit_t;

typedef enum bt_io_popn_light_side_lamp_bit {
    BT_IO_POPN_LIGHT_LEFT_LAMP1 = 0x00,
    BT_IO_POPN_LIGHT_LEFT_LAMP2 = 0x01,
    BT_IO_POPN_LIGHT_RIGHT_LAMP1 = 0x02,
    BT_IO_POPN_LIGHT_RIGHT_LAMP2 = 0x03,
} bt_io_popn_light_side_lamp_bit_t;

typedef enum bt_io_popn_light_switch_lamp_bit {
    BT_IO_POPN_LIGHT_SW_LAMP1 = 0x00, // Is actually 4 bits wide
    BT_IO_POPN_LIGHT_SW_LAMP2 = 0x04,
    BT_IO_POPN_LIGHT_SW_LAMP3 = 0x05,
    BT_IO_POPN_LIGHT_SW_LAMP4 = 0x06,
    BT_IO_POPN_LIGHT_SW_LAMP5 = 0x07,
    BT_IO_POPN_LIGHT_SW_LAMP6 = 0x08,
    BT_IO_POPN_LIGHT_SW_LAMP7 = 0x09,
    BT_IO_POPN_LIGHT_SW_LAMP8 = 0x0a,
    BT_IO_POPN_LIGHT_SW_LAMP9 = 0x0b,
} bt_io_popn_light_switch_lamp_bit_t;

typedef bool (*bt_io_popn_init_t)();
typedef void (*bt_io_popn_fini_t)();
typedef uint32_t (*bt_io_popn_buttons_get_t)();
typedef void (*bt_io_popn_top_lights_set_t)(uint32_t lights);
typedef void (*bt_io_popn_side_lights_set_t)(uint32_t lights);
typedef void (*bt_io_popn_button_lights_set_t)(uint32_t lights);
typedef void (*bt_io_popn_coin_blocker_light_set_t)(bool enabled);
typedef void (*bt_io_popn_coin_counter_light_set_t)(bool enabled);

typedef struct bt_io_popn_api {
    uint16_t version;

    struct {
        // Required to be implemented
        bt_io_popn_init_t init;
        bt_io_popn_fini_t fini;
        bt_io_popn_buttons_get_t buttons_get;
        bt_io_popn_top_lights_set_t top_lights_set;
        bt_io_popn_side_lights_set_t side_lights_set;
        bt_io_popn_button_lights_set_t button_lights_set;
        bt_io_popn_coin_blocker_light_set_t coin_blocker_light_set;
        bt_io_popn_coin_counter_light_set_t coin_counter_light_set;
    } v1;
} bt_io_popn_api_t;

#endif

#ifndef BT_IO_POPN_H
#define BT_IO_POPN_H

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

/* Initialize your pop'n music IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool bt_io_popn_init();

/* Shut down your pop'n music IO emulation DLL */

void bt_io_popn_fini();

/* Get the state of the buttons. See enums above. */

uint32_t bt_io_popn_buttons_get();

/* Set the top light state. See enum bt_io_popn_light_top_lamp_bit above. */

void bt_io_popn_top_lights_set(uint32_t lights);

/* Set the side light state. See enum bt_io_popn_light_side_lamp_bit above. */

void bt_io_popn_side_lights_set(uint32_t lights);

/* Set the button light state. See enum bt_io_popn_light_switch_lamp_bit above. */

void bt_io_popn_button_lights_set(uint32_t lights);

/* Set the coin blocker light state. */

void bt_io_popn_coin_blocker_light_set(bool enabled);

/* Set the coin counter light state. */

void bt_io_popn_coin_counter_light_set(bool enabled);

#endif

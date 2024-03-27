#ifndef BEMANITOOLS_POPNIO_H
#define BEMANITOOLS_POPNIO_H

/* IO emulation provider for pop'n music. */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

/* Bit mapping for the "buttons" word */

enum popn_io_sys_bit {
    POPN_IO_SYS_SERVICE = 0x06,
    POPN_IO_SYS_TEST = 0x07,
};

enum popn_io_button_bit {
    POPN_IO_BUTTON_1 = 0x08,
    POPN_IO_BUTTON_2 = 0x09,
    POPN_IO_BUTTON_3 = 0x0a,
    POPN_IO_BUTTON_4 = 0x0b,
    POPN_IO_BUTTON_5 = 0x0c,
    POPN_IO_BUTTON_6 = 0x0d,
    POPN_IO_BUTTON_7 = 0x0e,
    POPN_IO_BUTTON_8 = 0x0f,
    POPN_IO_BUTTON_9 = 0x10,
};

enum popn_io_light_top_lamp_bit {
    POPN_LIGHT_HI_LAMP1 = 0x00,
    POPN_LIGHT_HI_LAMP2 = 0x01,
    POPN_LIGHT_HI_LAMP3 = 0x02,
    POPN_LIGHT_HI_LAMP4 = 0x03,
    POPN_LIGHT_HI_LAMP5 = 0x04,
};

enum popn_io_light_side_lamp_bit {
    POPN_LIGHT_LEFT_LAMP1 = 0x00,
    POPN_LIGHT_LEFT_LAMP2 = 0x01,
    POPN_LIGHT_RIGHT_LAMP1 = 0x02,
    POPN_LIGHT_RIGHT_LAMP2 = 0x03,
};

enum popn_io_light_switch_lamp_bit {
    POPN_LIGHT_SW_LAMP1 = 0x00, // Is actually 4 bits wide
    POPN_LIGHT_SW_LAMP2 = 0x04,
    POPN_LIGHT_SW_LAMP3 = 0x05,
    POPN_LIGHT_SW_LAMP4 = 0x06,
    POPN_LIGHT_SW_LAMP5 = 0x07,
    POPN_LIGHT_SW_LAMP6 = 0x08,
    POPN_LIGHT_SW_LAMP7 = 0x09,
    POPN_LIGHT_SW_LAMP8 = 0x0a,
    POPN_LIGHT_SW_LAMP9 = 0x0b,
};

/* The first function that will be called on your DLL. You will be supplied
   with four function pointers that may be used to log messages to the game's
   log file. See comments in glue.h for further information. */

void popn_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/* Initialize your pop'n music IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool popn_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down your pop'n music IO emulation DLL */

void popn_io_fini(void);

/* Get the state of the buttons. See enums above. */

uint32_t popn_io_get_buttons(void);

/* Set the top light state. See enum popn_io_light_top_lamp_bit above. */

void popn_io_set_top_lights(uint32_t lights);

/* Set the side light state. See enum popn_io_light_side_lamp_bit above. */

void popn_io_set_side_lights(uint32_t lights);

/* Set the button light state. See enum popn_io_light_switch_lamp_bit above. */

void popn_io_set_button_lights(uint32_t lights);

/* Set the coin blocker light state. */

void popn_io_set_coin_blocker_light(bool enabled);

/* Set the coin counter light state. */

void popn_io_set_coin_counter_light(bool enabled);

#endif

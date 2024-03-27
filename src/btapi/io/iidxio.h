#ifndef BEMANITOOLS_IIDXIO_H
#define BEMANITOOLS_IIDXIO_H

/* IO emulation provider for beatmania IIDX. */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

/* Bit mapping for the "pad" word */

enum iidx_io_sys_bit {
    IIDX_IO_SYS_TEST = 0x00,
    IIDX_IO_SYS_SERVICE = 0x01,
    IIDX_IO_SYS_COIN = 0x02
};

enum iidx_io_panel_bit {
    IIDX_IO_PANEL_P1_START = 0x00,
    IIDX_IO_PANEL_P2_START = 0x01,
    IIDX_IO_PANEL_VEFX = 0x02,
    IIDX_IO_PANEL_EFFECT = 0x03
};

enum iidx_io_key_bit {
    IIDX_IO_KEY_P1_1 = 0x00,
    IIDX_IO_KEY_P1_2 = 0x01,
    IIDX_IO_KEY_P1_3 = 0x02,
    IIDX_IO_KEY_P1_4 = 0x03,
    IIDX_IO_KEY_P1_5 = 0x04,
    IIDX_IO_KEY_P1_6 = 0x05,
    IIDX_IO_KEY_P1_7 = 0x06,

    IIDX_IO_KEY_P2_1 = 0x07,
    IIDX_IO_KEY_P2_2 = 0x08,
    IIDX_IO_KEY_P2_3 = 0x09,
    IIDX_IO_KEY_P2_4 = 0x0A,
    IIDX_IO_KEY_P2_5 = 0x0B,
    IIDX_IO_KEY_P2_6 = 0x0C,
    IIDX_IO_KEY_P2_7 = 0x0D
};

/* Bit mapping for the P1 and P2 deck lights */

enum iidx_io_deck_light {
    IIDX_IO_DECK_LIGHT_P1_1 = 0,
    IIDX_IO_DECK_LIGHT_P1_2 = 1,
    IIDX_IO_DECK_LIGHT_P1_3 = 2,
    IIDX_IO_DECK_LIGHT_P1_4 = 3,
    IIDX_IO_DECK_LIGHT_P1_5 = 4,
    IIDX_IO_DECK_LIGHT_P1_6 = 5,
    IIDX_IO_DECK_LIGHT_P1_7 = 6,

    IIDX_IO_DECK_LIGHT_P2_1 = 8,
    IIDX_IO_DECK_LIGHT_P2_2 = 9,
    IIDX_IO_DECK_LIGHT_P2_3 = 10,
    IIDX_IO_DECK_LIGHT_P2_4 = 11,
    IIDX_IO_DECK_LIGHT_P2_5 = 12,
    IIDX_IO_DECK_LIGHT_P2_6 = 13,
    IIDX_IO_DECK_LIGHT_P2_7 = 14,
};

/* Bit mapping for the front panel lights */

enum iidx_io_panel_light {
    IIDX_IO_PANEL_LIGHT_P1_START = 0,
    IIDX_IO_PANEL_LIGHT_P2_START = 1,
    IIDX_IO_PANEL_LIGHT_VEFX = 2,
    IIDX_IO_PANEL_LIGHT_EFFECT = 3,
};

/* Bit mapping for the top lamps from left to right when facing cabinet screen
 */

enum iidx_io_top_lamp {
    IIDX_IO_TOP_LAMP_LEFT_BLUE = 0,
    IIDX_IO_TOP_LAMP_LEFT_GREEN = 1,
    IIDX_IO_TOP_LAMP_LEFT_YELLOW = 2,
    IIDX_IO_TOP_LAMP_LEFT_RED = 3,
    IIDX_IO_TOP_LAMP_RIGHT_BLUE = 4,
    IIDX_IO_TOP_LAMP_RIGHT_GREEN = 5,
    IIDX_IO_TOP_LAMP_RIGHT_YELLOW = 6,
    IIDX_IO_TOP_LAMP_RIGHT_RED = 7,
};

/* The first function that will be called on your DLL. You will be supplied
   with four function pointers that may be used to log messages to the game's
   log file. See comments in glue.h for further information. */

void iidx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/* Initialize your IIDX IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool iidx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down your IIDX IO emulation DLL */

void iidx_io_fini(void);

/* Set the deck lighting state. See enum iidx_io_deck_light above. */

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights);

/* Set front panel lighting state. See enum iidx_io_panel_light above. */

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights);

/* Set state of the eight halogens above the marquee. */

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps);

/* Switch the top neons on or off. */

void iidx_io_ep1_set_top_neons(bool top_neons);

/* Transmit the lighting state to the lighting controller. This function is
   called immediately after all of the other iidx_io_ep1_set_*() functions.

   Return false in the event of an IO error. This will lock the game into an
   IO error screen. */

bool iidx_io_ep1_send(void);

/* Read input state from the input controller. This function is called
   immediately before all of the iidx_io_ep2_get_*() functions.

   Return false in the event of an IO error. This will lock the game into an
   IO error screen. */

bool iidx_io_ep2_recv(void);

/* Get absolute turntable position, expressed in 1/256ths of a rotation.
   player_no is either 0 or 1. */

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no);

/* Get slider position, where 0 is the bottom position and 15 is the topmost
   position. slider_no is a number between 0 (leftmost) and 4 (rightmost). */

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no);

/* Get the state of the system buttons. See enums above. */

uint8_t iidx_io_ep2_get_sys(void);

/* Get the state of the panel buttons. See enums above. */

uint8_t iidx_io_ep2_get_panel(void);

/* Get the state of the 14 key buttons. See enums above. */

uint16_t iidx_io_ep2_get_keys(void);

/* Write a nine-character string to the 16-segment display. This happens on a
   different schedule to all of the other IO operations, so you should initiate
   the communication as soon as this function is called */

bool iidx_io_ep3_write_16seg(const char *text);

#endif

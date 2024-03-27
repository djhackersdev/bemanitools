#ifndef BT_IO_IIDX_H
#define BT_IO_IIDX_H

/* IO emulation provider for beatmania IIDX. */

#include <stdbool.h>
#include <stdint.h>

/* Bit mapping for the "pad" word */

typedef enum bt_io_iidx_sys_bit {
    BT_IO_IIDX_SYS_TEST = 0x00,
    BT_IO_IIDX_SYS_SERVICE = 0x01,
    BT_IO_IIDX_SYS_COIN = 0x02
} bt_io_iidx_sys_bit_t;

typedef enum bt_io_iidx_panel_bit {
    BT_IO_IIDX_PANEL_P1_START = 0x00,
    BT_IO_IIDX_PANEL_P2_START = 0x01,
    BT_IO_IIDX_PANEL_VEFX = 0x02,
    BT_IO_IIDX_PANEL_EFFECT = 0x03
} bt_io_iidx_panel_bit_t;

typedef enum bt_io_iidx_key_bit {
    BT_IO_IIDX_KEY_P1_1 = 0x00,
    BT_IO_IIDX_KEY_P1_2 = 0x01,
    BT_IO_IIDX_KEY_P1_3 = 0x02,
    BT_IO_IIDX_KEY_P1_4 = 0x03,
    BT_IO_IIDX_KEY_P1_5 = 0x04,
    BT_IO_IIDX_KEY_P1_6 = 0x05,
    BT_IO_IIDX_KEY_P1_7 = 0x06,

    BT_IO_IIDX_KEY_P2_1 = 0x07,
    BT_IO_IIDX_KEY_P2_2 = 0x08,
    BT_IO_IIDX_KEY_P2_3 = 0x09,
    BT_IO_IIDX_KEY_P2_4 = 0x0A,
    BT_IO_IIDX_KEY_P2_5 = 0x0B,
    BT_IO_IIDX_KEY_P2_6 = 0x0C,
    BT_IO_IIDX_KEY_P2_7 = 0x0D
} bt_io_iidx_key_bit_t;

/* Bit mapping for the P1 and P2 deck lights */

typedef enum bt_io_iidx_deck_light {
    BT_IO_IIDX_DECK_LIGHT_P1_1 = 0,
    BT_IO_IIDX_DECK_LIGHT_P1_2 = 1,
    BT_IO_IIDX_DECK_LIGHT_P1_3 = 2,
    BT_IO_IIDX_DECK_LIGHT_P1_4 = 3,
    BT_IO_IIDX_DECK_LIGHT_P1_5 = 4,
    BT_IO_IIDX_DECK_LIGHT_P1_6 = 5,
    BT_IO_IIDX_DECK_LIGHT_P1_7 = 6,

    BT_IO_IIDX_DECK_LIGHT_P2_1 = 8,
    BT_IO_IIDX_DECK_LIGHT_P2_2 = 9,
    BT_IO_IIDX_DECK_LIGHT_P2_3 = 10,
    BT_IO_IIDX_DECK_LIGHT_P2_4 = 11,
    BT_IO_IIDX_DECK_LIGHT_P2_5 = 12,
    BT_IO_IIDX_DECK_LIGHT_P2_6 = 13,
    BT_IO_IIDX_DECK_LIGHT_P2_7 = 14,
} bt_io_iidx_deck_light_t;

/* Bit mapping for the front panel lights */

typedef enum bt_io_iidx_panel_light {
    BT_IO_IIDX_PANEL_LIGHT_P1_START = 0,
    BT_IO_IIDX_PANEL_LIGHT_P2_START = 1,
    BT_IO_IIDX_PANEL_LIGHT_VEFX = 2,
    BT_IO_IIDX_PANEL_LIGHT_EFFECT = 3,
} bt_io_iidx_panel_light_t;

/* Bit mapping for the top lamps from left to right when facing cabinet screen
 */

typedef enum bt_io_iidx_top_lamp {
    BT_IO_IIDX_TOP_LAMP_LEFT_BLUE = 0,
    BT_IO_IIDX_TOP_LAMP_LEFT_GREEN = 1,
    BT_IO_IIDX_TOP_LAMP_LEFT_YELLOW = 2,
    BT_IO_IIDX_TOP_LAMP_LEFT_RED = 3,
    BT_IO_IIDX_TOP_LAMP_RIGHT_BLUE = 4,
    BT_IO_IIDX_TOP_LAMP_RIGHT_GREEN = 5,
    BT_IO_IIDX_TOP_LAMP_RIGHT_YELLOW = 6,
    BT_IO_IIDX_TOP_LAMP_RIGHT_RED = 7,
} bt_io_iidx_top_lamp_t;

typedef bool (*bt_io_iidx_init_t)();
typedef void (*bt_io_iidx_fini_t)();
typedef void (*bt_io_iidx_ep1_deck_lights_set_t)(uint16_t deck_lights);
typedef void (*bt_io_iidx_ep1_panel_lights_set_t)(uint8_t panel_lights);
typedef void (*bt_io_iidx_ep1_top_lamps_set_t)(uint8_t top_lamps);
typedef void (*bt_io_iidx_ep1_top_neons_set_t)(bool top_neons);
typedef bool (*bt_io_iidx_ep1_send_t)();
typedef bool (*bt_io_iidx_ep2_recv_t)();
typedef uint8_t (*bt_io_iidx_ep2_turntable_get_t)(uint8_t player_no);
typedef uint8_t (*bt_io_iidx_ep2_slider_get_t)(uint8_t slider_no);
typedef uint8_t (*bt_io_iidx_ep2_sys_get_t)();
typedef uint8_t (*bt_io_iidx_ep2_panel_get_t)();
typedef uint16_t (*bt_io_iidx_ep2_keys_get_t)();
typedef bool (*bt_io_iidx_ep3_16seg_send_t)(const char *text);

/* Initialize your IIDX IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool bt_io_iidx_init();

/* Shut down your IIDX IO emulation DLL */

void bt_io_iidx_fini();

/* Set the deck lighting state. See enum bt_io_iidx_deck_light above. */

void bt_io_iidx_ep1_deck_lights_set(uint16_t deck_lights);

/* Set front panel lighting state. See enum bt_io_iidx_panel_light above. */

void bt_io_iidx_ep1_panel_lights_set(uint8_t panel_lights);

/* Set state of the eight halogens above the marquee. */

void bt_io_iidx_ep1_top_lamps_set(uint8_t top_lamps);

/* Switch the top neons on or off. */

void bt_io_iidx_ep1_top_neons_set(bool top_neons);

/* Transmit the lighting state to the lighting controller. This function is
   called immediately after all of the other bt_io_iidx_ep1_set_*() functions.

   Return false in the event of an IO error. This will lock the game into an
   IO error screen. */

bool bt_io_iidx_ep1_send();

/* Read input state from the input controller. This function is called
   immediately before all of the bt_io_iidx_ep2_get_*() functions.

   Return false in the event of an IO error. This will lock the game into an
   IO error screen. */

bool bt_io_iidx_ep2_recv();

/* Get absolute turntable position, expressed in 1/256ths of a rotation.
   player_no is either 0 or 1. */

uint8_t bt_io_iidx_ep2_turntable_get(uint8_t player_no);

/* Get slider position, where 0 is the bottom position and 15 is the topmost
   position. slider_no is a number between 0 (leftmost) and 4 (rightmost). */

uint8_t bt_io_iidx_ep2_slider_get(uint8_t slider_no);

/* Get the state of the system buttons. See enums above. */

uint8_t bt_io_iidx_ep2_sys_get();

/* Get the state of the panel buttons. See enums above. */

uint8_t bt_io_iidx_ep2_panel_get();

/* Get the state of the 14 key buttons. See enums above. */

uint16_t bt_io_iidx_ep2_keys_get();

/* Write a nine-character string to the 16-segment display. This happens on a
   different schedule to all of the other IO operations, so you should initiate
   the communication as soon as this function is called */

bool bt_io_iidx_ep3_16seg_send(const char *text);

#endif

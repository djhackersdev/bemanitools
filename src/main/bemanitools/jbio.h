#ifndef BEMANITOOLS_JBIO_H
#define BEMANITOOLS_JBIO_H

/* IO emulation provider for jubeat. */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

/* input bit mappings. Panels on the controller are
    panel 1 top left corner down to panel 16 bottom right corner */
enum jb_io_panel_bit {
    JB_IO_PANEL_01 = 0x00,
    JB_IO_PANEL_02 = 0x01,
    JB_IO_PANEL_03 = 0x02,
    JB_IO_PANEL_04 = 0x03,
    JB_IO_PANEL_05 = 0x04,
    JB_IO_PANEL_06 = 0x05,
    JB_IO_PANEL_07 = 0x06,
    JB_IO_PANEL_08 = 0x07,
    JB_IO_PANEL_09 = 0x08,
    JB_IO_PANEL_10 = 0x09,
    JB_IO_PANEL_11 = 0x0A,
    JB_IO_PANEL_12 = 0x0B,
    JB_IO_PANEL_13 = 0x0C,
    JB_IO_PANEL_14 = 0x0D,
    JB_IO_PANEL_15 = 0x0E,
    JB_IO_PANEL_16 = 0x0F,
};

/* input "single button mode" mappings. Allows you to check each corner of each
    button to determine any flaky inputs
*/
enum jb_io_panel_mode {
    JB_IO_PANEL_MODE_ALL = 0, // any of the four corners will trigger a panel
    JB_IO_PANEL_MODE_TOP_LEFT = 1,
    JB_IO_PANEL_MODE_TOP_RIGHT = 2,
    JB_IO_PANEL_MODE_BOTTOM_RIGHT = 3,
    JB_IO_PANEL_MODE_BOTTOM_LEFT = 4,
};

/* Bit mappings for "system" inputs */
enum jb_io_sys_bit {
    JB_IO_SYS_TEST = 0x00,
    JB_IO_SYS_SERVICE = 0x01,
    JB_IO_SYS_COIN = 0x02,
};

/* RGB led units to address */
enum jb_io_rgb_led {
    JB_IO_RGB_LED_FRONT = 0,
    JB_IO_RGB_LED_TOP = 1,
    JB_IO_RGB_LED_LEFT = 2,
    JB_IO_RGB_LED_RIGHT = 3,
    JB_IO_RGB_LED_TITLE = 4,
    JB_IO_RGB_LED_WOOFER = 5
};

/* The first function that will be called on your DLL. You will be supplied
   with four function pointers that may be used to log messages to the game's
   log file. See comments in glue.h for further information. */

void jb_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/* Initialize your JB IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   jb_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool jb_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down your JB IO emulation DLL */

void jb_io_fini(void);

/* Read input state */

bool jb_io_read_inputs(void);

/* Get state of coin, test, service inputs */

uint8_t jb_io_get_sys_inputs(void);

/* Get panel button state. Will return either any button being pressed, or a
   particular panel corner depending on previous call to jb_io_set_panel_mode */

uint16_t jb_io_get_panel_inputs(void);

/* Set state of a PWM (dimmable) light */

void jb_io_set_rgb_led(
    enum jb_io_rgb_led unit, uint8_t r, uint8_t g, uint8_t b);

/* Transmit the light state to the IOPCB */

bool jb_io_write_lights(void);

/* Select operating mode for the panel. Should be immediately sent to the IOPCB
 */

bool jb_io_set_panel_mode(enum jb_io_panel_mode mode);

/* Open or close the coin chute - true will redirect all coins to the return
   slot. Required for jb_io_set_panel_mode to operate correctly on p4io.
   Should be immediately sent to the IOPCB */

bool jb_io_set_coin_blocker(bool blocked);

#endif

#ifndef BT_IO_VEFX_H
#define BT_IO_VEFX_H

/* IO emulation provider for beatmania IIDX Effector Panel. */

#include <stdbool.h>
#include <stdint.h>

typedef bool (*bt_io_vefx_init_t)();
typedef void (*bt_io_vefx_fini_t)();
typedef bool (*bt_io_vefx_recv_t)(uint64_t *ppad);
typedef uint8_t (*bt_io_vefx_slider_get_t)(uint8_t slider_no);
typedef bool (*bt_io_vefx_16seg_send_t)(const char *text);

/* Initialize your IIDX IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool bt_io_vefx_init();

/* Shut down your IIDX IO emulation DLL */

void bt_io_vefx_fini();

/* Read input state from the input controller. This function is called
   immediately before the bt_io_vefx_get_slider() function.

   Return false in the event of an IO error. This will lock the game into an
   IO error screen.

   If making a custom driver, ppad can be used to update regular IO if needed

   See iidxio.c for mappings. */

bool bt_io_vefx_recv(uint64_t *ppad);

/* Get slider position, where 0 is the bottom position and 15 is the topmost
   position. slider_no is a number between 0 (leftmost) and 4 (rightmost). */

uint8_t bt_io_vefx_slider_get(uint8_t slider_no);

/* Write a nine-character string to the 16-segment display. This happens on a
   different schedule to all of the other IO operations, so you should initiate
   the communication as soon as this function is called */

bool bt_io_vefx_16seg_send(const char *text);

#endif

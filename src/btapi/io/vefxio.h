#ifndef BEMANITOOLS_VEFXIO_H
#define BEMANITOOLS_VEFXIO_H

/* IO emulation provider for beatmania IIDX Effector Panel. */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

/* The first function that will be called on your DLL. You will be supplied
   with four function pointers that may be used to log messages to the game's
   log file. See comments in glue.h for further information. */

void vefx_io_set_loggers(
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

bool vefx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down your IIDX IO emulation DLL */

void vefx_io_fini(void);

/* Read input state from the input controller. This function is called
   immediately before the vefx_io_get_slider() function.

   Return false in the event of an IO error. This will lock the game into an
   IO error screen.

   If making a custom driver, ppad can be used to update regular IO if needed

   See iidxio.c for mappings. */

bool vefx_io_recv(uint64_t *ppad);

/* Get slider position, where 0 is the bottom position and 15 is the topmost
   position. slider_no is a number between 0 (leftmost) and 4 (rightmost). */

uint8_t vefx_io_get_slider(uint8_t slider_no);

/* Write a nine-character string to the 16-segment display. This happens on a
   different schedule to all of the other IO operations, so you should initiate
   the communication as soon as this function is called */

bool vefx_io_write_16seg(const char *text);

#endif

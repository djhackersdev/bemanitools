#ifndef BEMANITOOLS_SDVXIO_H
#define BEMANITOOLS_SDVXIO_H

/* IO emulation provider for BeatStream */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

enum bst_io_in_gpio_sys_bit {
    SDVX_IO_IN_GPIO_SYS_COIN = 2,
    SDVX_IO_IN_GPIO_SYS_TEST = 4,
    SDVX_IO_IN_GPIO_SYS_SERVICE = 5,
};

/* The first function that will be called on your DLL. You will be supplied
   with four function pointers that may be used to log messages to the game's
   log file. See comments in glue.h for further information. */

void bst_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/* Initialize your BST IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool bst_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down your SDVX IO emulation DLL */

void bst_io_fini(void);

/* Read input state. Returns true if successful. */

bool bst_io_read_input(void);

/* Get state of coin, test, service inputs */

uint8_t bst_io_get_input(void);

// TODO: Lighting

#endif

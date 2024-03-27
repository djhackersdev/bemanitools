#ifndef BEMANITOOLS_INPUT_H
#define BEMANITOOLS_INPUT_H

/* Generic input API. This header file defines the public API for geninput.dll.

   You may use geninput to supply generic input mapping services for controls
   that your custom IO DLLs do not natively provide. For instance, you might
   want to make a custom IIDXIO.DLL that interfaces with your own 16-segment
   LCD marquee device while still using the stock IIDXIO.DLL input and lighting
   code, which uses the generic services provided by geninput.dll.

   All other exports from geninput.dll are undocumented and subject to change
   without notice. */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

/* Supply logging functions to geninput. You should pass on the logging
   functions that are supplied to your own custom DLLs.

   This is the only function that can safely be called before input_init(). */

void input_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/* Initialize the generic input subsystem. You must pass on the thread
   management functions that have been supplied to your DLL.

   Calling any geninput functions other than input_set_loggers() before calling
   input_init() will probably crash the running process.

   You will also need to call mapper_config_load() with the appropriate
   game_type parameter, otherwise you will not receive any input, and any
   attempts to set a light output level will have no effect. */

void input_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down the generic input subsystem. After calling this function, no
   geninput functions other than input_set_loggers() or input_init() may be
   called. */

void input_fini(void);

/* Load input mappings for a particular game, as configured from config.exe.

   Currently recognized game types are:

   ddr:     Dance Dance Revolution
   dm:      Drum Mania
   gf:      Guitar Freaks
   iidx:    beatmania IIDX
   pnm:     pop'n music
   sdvx:    Sound Voltex
   ju:      jubeat

   Returns true if a suitable config file was found and successfully loaded. */

bool mapper_config_load(const char *game_type);

/* Return the absolute position of an analog spinner, expressed in 1/256ths of
   a complete rotation. */

uint8_t mapper_read_analog(uint8_t analog);

/* Map the current state of all attached input devices to a 64-bit bit field.
   The exact layout of this bit field varies between game types, although we
   try to approximate the contents of each emulated IO PCB's own state packet
   as closely as is reasonably practical. */

uint64_t mapper_update(void);

/* Set the intensity of any light on a controller corresponding to a particular
   software-controlled light on an arcade cabinet, where 0 is off and 255 is
   full intensity. Consult the header files for the light identifiers used for
   each game type. The mappings between these light identifiers and the actual
   lights on the user's controller (if any) are configured by the user by means
   of the config.exe program.

   Note that any calls to this function do not take effect until the next call
   to mapper_update(). */

void mapper_write_light(uint8_t light, uint8_t intensity);

#endif

#ifndef BT_INPUT_H
#define BT_INPUT_H

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

typedef bool (*bt_input_init_t)();
typedef void (*bt_input_fini_t)();
typedef bool (*bt_input_mapper_config_load_t)(const char *game_type);
typedef uint8_t (*bt_input_mapper_analog_read_t)(uint8_t analog);
typedef uint64_t (*bt_input_mapper_update_t)();
typedef void (*bt_input_mapper_light_write_t)(uint8_t light, uint8_t intensity);

/* Initialize the generic input subsystem. You must pass on the thread
   management functions that have been supplied to your DLL.

   Calling any geninput functions other than input_set_loggers() before calling
   input_init() will probably crash the running process.

   You will also need to call mapper_config_load() with the appropriate
   game_type parameter, otherwise you will not receive any input, and any
   attempts to set a light output level will have no effect. */

bool bt_input_init();

/* Shut down the generic input subsystem. After calling this function, no
   geninput functions other than input_set_loggers() or input_init() may be
   called. */

void bt_input_fini();

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

bool bt_input_mapper_config_load(const char *game_type);

/* Return the absolute position of an analog spinner, expressed in 1/256ths of
   a complete rotation. */

uint8_t bt_input_mapper_analog_read(uint8_t analog);

/* Map the current state of all attached input devices to a 64-bit bit field.
   The exact layout of this bit field varies between game types, although we
   try to approximate the contents of each emulated IO PCB's own state packet
   as closely as is reasonably practical. */

uint64_t bt_input_mapper_update();

/* Set the intensity of any light on a controller corresponding to a particular
   software-controlled light on an arcade cabinet, where 0 is off and 255 is
   full intensity. Consult the header files for the light identifiers used for
   each game type. The mappings between these light identifiers and the actual
   lights on the user's controller (if any) are configured by the user by means
   of the config.exe program.

   Note that any calls to this function do not take effect until the next call
   to mapper_update(). */

void bt_input_mapper_light_write(uint8_t light, uint8_t intensity);

#endif

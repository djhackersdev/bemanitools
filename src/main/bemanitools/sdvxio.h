#ifndef BEMANITOOLS_SDVXIO_H
#define BEMANITOOLS_SDVXIO_H

/* IO emulation provider for SOUND VOLTEX */

#include <stdbool.h>
#include <stdint.h>

#include "bemanitools/glue.h"

enum sdvx_io_in_gpio_sys_bit {
    SDVX_IO_IN_GPIO_SYS_COIN = 2,
    SDVX_IO_IN_GPIO_SYS_SERVICE = 4,
    SDVX_IO_IN_GPIO_SYS_TEST = 5,
};

enum sdvx_io_in_gpio_0_bit {
    SDVX_IO_IN_GPIO_0_C = 0,
    SDVX_IO_IN_GPIO_0_B = 1,
    SDVX_IO_IN_GPIO_0_A = 2,
    SDVX_IO_IN_GPIO_0_START = 3,
    SDVX_IO_IN_GPIO_0_RECORDER = 4,
    SDVX_IO_IN_GPIO_0_HEADPHONE = 5,
};

enum sdvx_io_in_gpio_1_bit {
    SDVX_IO_IN_GPIO_1_FX_R = 3,
    SDVX_IO_IN_GPIO_1_FX_L = 4,
    SDVX_IO_IN_GPIO_1_D = 5,
};

enum sdvx_io_out_gpio_bit {
    SDVX_IO_OUT_GPIO_D = 0,
    SDVX_IO_OUT_GPIO_FX_L = 1,
    SDVX_IO_OUT_GPIO_FX_R = 2,
    SDVX_IO_OUT_GPIO_GENERATOR_B = 3,
    SDVX_IO_OUT_GPIO_START = 12,
    SDVX_IO_OUT_GPIO_A = 13,
    SDVX_IO_OUT_GPIO_B = 14,
    SDVX_IO_OUT_GPIO_C = 15,
};

/* The first function that will be called on your DLL. You will be supplied
   with four function pointers that may be used to log messages to the game's
   log file. See comments in glue.h for further information. */

void sdvx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

/* Initialize your SDVX IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool sdvx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

/* Shut down your SDVX IO emulation DLL */

void sdvx_io_fini(void);

/* Set state of the GPIO (on/off) lights (see bit definitions above) */

void sdvx_io_set_gpio_lights(uint32_t gpio_lights);

/* Set state of a PWM (dimmable) light channel. These come in groups of three
   (red, green, blue). There are a six group of three PWM channels, for a
   total of 18 channels (0 through 17). */

void sdvx_io_set_pwm_light(uint8_t light_no, uint8_t intensity);

/* Transmit the light state to the IOPCB */

bool sdvx_io_write_output(void);

/* Read input state */

bool sdvx_io_read_input(void);

/* Get state of coin, test, service inputs */

uint8_t sdvx_io_get_input_gpio_sys(void);

/* Get gameplay button state. Parameter selects GPIO bank 0 or 1. See bit
   definitions above for details. */

uint16_t sdvx_io_get_input_gpio(uint8_t gpio_bank);

/* Get a 10-bit (!) spinner position, where spinner_no is 0 or 1.
   High six bits are ignored. */

uint16_t sdvx_io_get_spinner_pos(uint8_t spinner_no);

/* Sets the volume of the digital amps if possible.
   Range is between 0-96, where 0 is MAX and 96 is LOW. */

bool sdvx_io_set_amp_volume(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer);

#endif

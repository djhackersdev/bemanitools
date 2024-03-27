#ifndef BT_IO_SDVX_H
#define BT_IO_SDVX_H

/* IO emulation provider for SOUND VOLTEX */

#include <stdbool.h>
#include <stdint.h>

typedef enum bt_io_sdvx_in_gpio_sys_bit {
    BT_IO_SDVX_IN_GPIO_SYS_COIN = 2,
    BT_IO_SDVX_IN_GPIO_SYS_SERVICE = 4,
    BT_IO_SDVX_IN_GPIO_SYS_TEST = 5,
} bt_io_sdvx_in_gpio_sys_bit_t;

typedef enum bt_io_sdvx_in_gpio_0_bit {
    BT_IO_SDVX_IN_GPIO_0_C = 0,
    BT_IO_SDVX_IN_GPIO_0_B = 1,
    BT_IO_SDVX_IN_GPIO_0_A = 2,
    BT_IO_SDVX_IN_GPIO_0_START = 3,
    BT_IO_SDVX_IN_GPIO_0_RECORDER = 4,
    BT_IO_SDVX_IN_GPIO_0_HEADPHONE = 5,
} bt_io_sdvx_in_gpio_0_bit_t;

typedef enum bt_io_sdvx_in_gpio_1_bit {
    BT_IO_SDVX_IN_GPIO_1_FX_R = 3,
    BT_IO_SDVX_IN_GPIO_1_FX_L = 4,
    BT_IO_SDVX_IN_GPIO_1_D = 5,
} bt_io_sdvx_in_gpio_1_bit_t;

typedef enum bt_io_sdvx_out_gpio_bit {
    BT_IO_SDVX_OUT_GPIO_D = 0,
    BT_IO_SDVX_OUT_GPIO_FX_L = 1,
    BT_IO_SDVX_OUT_GPIO_FX_R = 2,
    BT_IO_SDVX_OUT_GPIO_GENERATOR_B = 3,
    BT_IO_SDVX_OUT_GPIO_START = 12,
    BT_IO_SDVX_OUT_GPIO_A = 13,
    BT_IO_SDVX_OUT_GPIO_B = 14,
    BT_IO_SDVX_OUT_GPIO_C = 15,
} bt_io_sdvx_out_gpio_bit_t;

typedef bool (*bt_io_sdvx_init_t)();
typedef void (*bt_io_sdvx_fini_t)();
typedef void (*bt_io_sdvx_gpio_lights_set_t)(uint32_t gpio_lights);
typedef void (*bt_io_sdvx_pwm_light_set_t)(uint8_t light_no, uint8_t intensity);
typedef bool (*bt_io_sdvx_output_write_t)();
typedef bool (*bt_io_sdvx_input_read_t)();
typedef uint8_t (*bt_io_sdvx_input_gpio_sys_get_t)();
typedef uint16_t (*bt_io_sdvx_input_gpio_get_t)(uint8_t gpio_bank);
typedef uint16_t (*bt_io_sdvx_spinner_pos_get_t)(uint8_t spinner_no);
typedef bool (*bt_io_sdvx_amp_volume_set_t)(uint8_t primary, uint8_t headphone, uint8_t subwoofer);

/* Initialize your SDVX IO emulation DLL. Thread management functions are
   provided to you; you must use these functions to create your own threads if
   you want to make use of the logging functions that are provided to
   eam_io_set_loggers(). You will also need to pass these thread management
   functions on to geninput if you intend to make use of that library.

   See glue.h and geninput.h for further details. */

bool bt_io_sdvx_init();

/* Shut down your SDVX IO emulation DLL */

void bt_io_sdvx_fini();

/* Set state of the GPIO (on/off) lights (see bit definitions above) */

void bt_io_sdvx_gpio_lights_set(uint32_t gpio_lights);

/* Set state of a PWM (dimmable) light channel. These come in groups of three
   (red, green, blue). There are a six group of three PWM channels, for a
   total of 18 channels (0 through 17). */

void bt_io_sdvx_pwm_light_set(uint8_t light_no, uint8_t intensity);

/* Transmit the light state to the IOPCB */

bool bt_io_sdvx_output_write();

/* Read input state */

bool bt_io_sdvx_input_read();

/* Get state of coin, test, service inputs */

uint8_t bt_io_sdvx_input_gpio_sys_get();

/* Get gameplay button state. Parameter selects GPIO bank 0 or 1. See bit
   definitions above for details. */

uint16_t bt_io_sdvx_input_gpio_get(uint8_t gpio_bank);

/* Get a 10-bit (!) spinner position, where spinner_no is 0 or 1.
   High six bits are ignored. */

uint16_t bt_io_sdvx_spinner_pos_get(uint8_t spinner_no);

/* Sets the volume of the digital amps if possible.
   Range is between 0-96, where 0 is MAX and 96 is LOW. */

bool bt_io_sdvx_amp_volume_set(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer);

#endif

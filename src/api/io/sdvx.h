#ifndef BT_API_IO_SDVX_H
#define BT_API_IO_SDVX_H

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

typedef struct bt_io_sdvx_api {
    uint16_t version;

    struct {
        // Required to be implemented
        bt_io_sdvx_init_t init;
        bt_io_sdvx_fini_t fini;
        bt_io_sdvx_gpio_lights_set_t gpio_lights_set;
        bt_io_sdvx_pwm_light_set_t pwm_light_set;
        bt_io_sdvx_output_write_t output_write;
        bt_io_sdvx_input_read_t input_read;
        bt_io_sdvx_input_gpio_sys_get_t input_gpio_sys_get;
        bt_io_sdvx_input_gpio_get_t input_gpio_get;
        bt_io_sdvx_spinner_pos_get_t spinner_pos_get;
        bt_io_sdvx_amp_volume_set_t amp_volume_set;
    } v1;
} bt_io_sdvx_api_t;

#endif

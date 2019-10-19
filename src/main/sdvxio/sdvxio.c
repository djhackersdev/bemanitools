#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bemanitools/glue.h"
#include "bemanitools/input.h"
#include "bemanitools/sdvxio.h"

static uint16_t sdvx_io_gpio[2];
static uint8_t sdvx_io_gpio_sys;

void sdvx_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    input_set_loggers(misc, info, warning, fatal);
}

bool sdvx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    input_init(thread_create, thread_join, thread_destroy);
    mapper_config_load("sdvx");

    return true;
}

void sdvx_io_fini(void)
{
    input_fini();
}

void sdvx_io_set_gpio_lights(uint32_t gpio_lights)
{
    size_t i;

    for (i = 0; i < 16; i++) {
        if (gpio_lights & (1 << i)) {
            mapper_write_light(i, 255);
        } else {
            mapper_write_light(i, 0);
        }
    }
}

void sdvx_io_set_pwm_light(uint8_t light_no, uint8_t intensity)
{
    mapper_write_light(light_no + 0x10, intensity);
}

bool sdvx_io_write_output(void)
{
    return true;
}

bool sdvx_io_read_input(void)
{
    uint32_t pack;

    pack = mapper_update();

    sdvx_io_gpio_sys = pack & 0xFF;
    sdvx_io_gpio[0] = (pack >> 8) & 0x00FF;
    sdvx_io_gpio[1] = (pack >> 16) & 0x00FF;

    return 0;
}

uint8_t sdvx_io_get_input_gpio_sys(void)
{
    return sdvx_io_gpio_sys;
}

uint16_t sdvx_io_get_input_gpio(uint8_t gpio_bank)
{
    if (gpio_bank > 1) {
        return 0;
    }

    return sdvx_io_gpio[gpio_bank];
}

uint16_t sdvx_io_get_spinner_pos(uint8_t spinner_no)
{
    return mapper_read_analog(spinner_no) * 4;
}

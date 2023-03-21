#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bemanitools/input.h"
#include "bemanitools/popnio.h"

void popn_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    input_set_loggers(misc, info, warning, fatal);
}

bool popn_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    input_init(thread_create, thread_join, thread_destroy);
    mapper_config_load("pnm");

    return true;
}

void popn_io_fini(void)
{
    input_fini();
}

uint32_t popn_io_get_buttons(void)
{
    return (uint32_t) mapper_update();
}

void popn_io_set_top_lights(uint32_t lights)
{
    uint8_t i;

    for (i = 0; i < 5; i++) {
        mapper_write_light(0x20 + i, lights & (1 << i) ? 255 : 0);
    }
}

void popn_io_set_side_lights(uint32_t lights)
{
    uint8_t i;

    for (i = 0; i < 4; i++) {
        mapper_write_light(0x25 + i, lights & (1 << i) ? 255 : 0);
    }
}

void popn_io_set_button_lights(uint32_t lights)
{
    uint8_t i;

    // Special case for POPN_LIGHT_SW_LAMP1 which is 4 bits wide
    mapper_write_light(0x17, lights & 0xf ? 255 : 0);

    for (i = 0; i < 8; i++) {
        mapper_write_light(0x18 + i, lights & (1 << (i + 4)) ? 255 : 0);
    }
}

void popn_io_set_coin_blocker_light(bool enabled)
{
    // mapper_write_light(x, enabled ? 255 : 0);
}

void popn_io_set_coin_counter_light(bool enabled)
{
    // mapper_write_light(x, enabled ? 255 : 0);
}

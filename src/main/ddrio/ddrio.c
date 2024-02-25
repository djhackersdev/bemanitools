#include <windows.h>

#include "imports/avs.h"

#include "bemanitools/input.h"

#include "core/log.h"

void ddr_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    input_set_loggers(misc, info, warning, fatal);
}

bool ddr_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    input_init(thread_create, thread_join, thread_destroy);
    mapper_config_load("ddr");

    return true;
}

uint32_t ddr_io_read_pad(void)
{
    /* Sleep first: input is timestamped immediately AFTER the ioctl returns.

       Which is the right thing to do, for once. We sleep here because
       the game polls input in a tight loop. Can't complain, at there isn't
       an artificial limit on the poll frequency. */

    Sleep(1);

    return (uint32_t) mapper_update();
}

void ddr_io_set_lights_extio(uint32_t lights)
{
    uint8_t i;

    for (i = 0x0E; i <= 0x1E; i++) {
        mapper_write_light(i, lights & (1 << i) ? 255 : 0);
    }
}

void ddr_io_set_lights_p3io(uint32_t lights)
{
    uint8_t i;

    for (i = 0x00; i <= 0x07; i++) {
        mapper_write_light(i, lights & (1 << i) ? 255 : 0);
    }
}

void ddr_io_set_lights_hdxs_panel(uint32_t lights)
{
    uint8_t i;

    for (i = 0x08; i <= 0x0D; i++) {
        mapper_write_light(i, lights & (1 << i) ? 255 : 0);
    }
}

void ddr_io_set_lights_hdxs_rgb(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx < 4) {
        uint8_t base = 0x20 + idx * 3;
        mapper_write_light(base + 0, r);
        mapper_write_light(base + 1, g);
        mapper_write_light(base + 2, b);
    }
}

void ddr_io_fini(void)
{
    input_fini();
}

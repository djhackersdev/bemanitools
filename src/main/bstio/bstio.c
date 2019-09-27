#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bemanitools/bstio.h"
#include "bemanitools/glue.h"
#include "bemanitools/input.h"

static uint8_t bst_io_gpio_sys;

void bst_io_set_loggers(log_formatter_t misc, log_formatter_t info,
        log_formatter_t warning, log_formatter_t fatal)
{
    input_set_loggers(misc, info, warning, fatal);
}

bool bst_io_init(thread_create_t thread_create, thread_join_t thread_join,
        thread_destroy_t thread_destroy)
{
    input_init(thread_create, thread_join, thread_destroy);
    mapper_config_load("bst");

    return true;
}

void bst_io_fini(void)
{
    input_fini();
}

bool bst_io_read_input(void)
{
    bst_io_gpio_sys = mapper_update();

    return true;
}

uint8_t bst_io_get_input(void)
{
    return bst_io_gpio_sys;
}


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "btapi/io/bstio.h"
#include "btapi/io/input.h"

static uint8_t bst_io_gpio_sys;

void bst_io_set_loggers(
    bt_core_log_message_t misc,
    bt_core_log_message_t info,
    bt_core_log_message_t warning,
    bt_core_log_message_t fatal)
{
    input_set_loggers(misc, info, warning, fatal);
}

bool bst_io_init(
    bt_core_thread_create_impl_t thread_create,
    bt_core_thread_join_impl_t thread_join,
    bt_core_thread_destroy_impl_t thread_destroy)
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

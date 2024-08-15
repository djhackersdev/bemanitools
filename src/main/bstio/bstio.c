#define LOG_MODULE "bstio"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface/input.h"

#include "module/input.h"

#include "main/module/input-ext.h"
#include "main/module/input.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/input.h"
#include "sdk/module/io/bst.h"

static module_input_t *_bst_io_module_input;
static uint8_t _bst_io_gpio_sys;

bool bt_io_bst_init()
{
    bool result;
    bt_input_api_t input_api;

    module_input_ext_load_and_init("geninput.dll", &_bst_io_module_input);
    module_input_api_get(_bst_io_module_input, &input_api);
    bt_input_api_set(&input_api);

    result = bt_input_init();

    if (!result) {
        return false;
    }

    return bt_input_mapper_config_load("bst");
}

void bt_io_bst_fini()
{
    bt_input_fini();

    bt_input_api_clear();
    module_input_free(&_bst_io_module_input);
}

bool bt_io_bst_input_read()
{
    _bst_io_gpio_sys = bt_input_mapper_update();

    return true;
}

uint8_t bt_io_bst_input_get()
{
    return _bst_io_gpio_sys;
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_bst_api_get(bt_io_bst_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_bst_init;
    api->v1.fini = bt_io_bst_fini;
    api->v1.input_read = bt_io_bst_input_read;
    api->v1.input_get = bt_io_bst_input_get;
}
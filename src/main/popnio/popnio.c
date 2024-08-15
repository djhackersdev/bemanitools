#define LOG_MODULE "popnio"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "api/core/log.h"
#include "api/core/thread.h"

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface/input.h"

#include "main/module/input-ext.h"
#include "main/module/input.h"
#include "main/module/io-ext.h"
#include "main/module/io.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/input.h"
#include "sdk/module/io/popn.h"

static module_input_t *_popn_io_module_input;

static void _bt_io_popn_module_input_init(module_input_t **module)
{
    bt_input_api_t api;

    module_input_ext_load_and_init("geninput.dll", module);
    module_input_api_get(*module, &api);
    bt_input_api_set(&api);
}

bool bt_io_popn_init()
{
    bool result;

    _bt_io_popn_module_input_init(&_popn_io_module_input);

    timeBeginPeriod(1);

    result = bt_input_init();

    if (!result) {
        log_warning("Initializing input failed");
        return false;
    }

    return bt_input_mapper_config_load("pnm");
}

void bt_io_popn_fini(void)
{
    bt_input_fini();
    bt_input_api_clear();
    module_input_free(&_popn_io_module_input);

    timeEndPeriod(1);
}

uint32_t bt_io_popn_buttons_get(void)
{
    return (uint32_t) bt_input_mapper_update();
}

void bt_io_popn_top_lights_set(uint32_t lights)
{
    uint8_t i;

    for (i = 0; i < 5; i++) {
        bt_input_mapper_light_write(0x20 + i, lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_popn_side_lights_set(uint32_t lights)
{
    uint8_t i;

    for (i = 0; i < 4; i++) {
        bt_input_mapper_light_write(0x25 + i, lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_popn_button_lights_set(uint32_t lights)
{
    uint8_t i;

    // Special case for POPN_LIGHT_SW_LAMP1 which is 4 bits wide
    bt_input_mapper_light_write(0x17, lights & 0xf ? 255 : 0);

    for (i = 0; i < 8; i++) {
        bt_input_mapper_light_write(
            0x18 + i, lights & (1 << (i + 4)) ? 255 : 0);
    }
}

void bt_io_popn_coin_blocker_light_set(bool enabled)
{
    // bt_input_mapper_light_write(x, enabled ? 255 : 0);
}

void bt_io_popn_coin_counter_light_set(bool enabled)
{
    // bt_input_mapper_light_write(x, enabled ? 255 : 0);
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_popn_api_get(bt_io_popn_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_popn_init;
    api->v1.fini = bt_io_popn_fini;
    api->v1.buttons_get = bt_io_popn_buttons_get;
    api->v1.top_lights_set = bt_io_popn_top_lights_set;
    api->v1.side_lights_set = bt_io_popn_side_lights_set;
    api->v1.button_lights_set = bt_io_popn_button_lights_set;
    api->v1.coin_blocker_light_set = bt_io_popn_coin_blocker_light_set;
    api->v1.coin_counter_light_set = bt_io_popn_coin_counter_light_set;
}
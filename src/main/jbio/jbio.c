#define LOG_MODULE "jbio"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <mmsystem.h>
// clang-format on

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
#include "sdk/module/io/jb.h"

static module_input_t *_jb_io_module_input;

static uint16_t jb_io_panels;
static uint8_t jb_io_sys_buttons;

static void _bt_io_jb_module_input_init(module_input_t **module)
{
    bt_input_api_t api;

    module_input_ext_load_and_init("geninput.dll", module);
    module_input_api_get(*module, &api);
    bt_input_api_set(&api);
}

bool bt_io_jb_init()
{
    bool result;

    timeBeginPeriod(1);

    _bt_io_jb_module_input_init(&_jb_io_module_input);

    result = bt_input_init();

    if (!result) {
        log_warning("Initializing input failed");
        return false;
    }

    return bt_input_mapper_config_load("jb");
}

void bt_io_jb_fini()
{
    bt_input_fini();
    bt_input_api_clear();
    module_input_free(&_jb_io_module_input);

    timeEndPeriod(1);
}

bool bt_io_jb_inputs_read()
{
    uint32_t buttons;
    /* Sleep first: input is timestamped immediately AFTER the ioctl returns.

       Which is the right thing to do, for once. We sleep here because
       the game polls input in a tight loop. Can't complain, at there isn't
       an artificial limit on the poll frequency. */

    Sleep(1);

    /* Update all of our input state here. */

    buttons = (uint32_t) bt_input_mapper_update();

    /* Mask out the stuff provided by geninput and store the panel/button state
       for later retrieval via jb_io_get_buttons() */

    jb_io_panels = buttons & 0xFFFF;
    jb_io_sys_buttons = (buttons >> 16) & 0x03;

    return true;
}

bool bt_io_jb_lights_write()
{
    /* The generic input stack currently initiates lighting sends and input
       reads simultaneously, though this might change later. Perform all of our
       I/O immediately before reading out the inputs so that the input state is
       as fresh as possible. */

    return true;
}

uint8_t bt_io_jb_sys_inputs_get()
{
    return jb_io_sys_buttons;
}

uint16_t bt_io_jb_panel_inputs_get()
{
    return jb_io_panels;
}

bool bt_io_jb_panel_mode_set(bt_io_jb_panel_mode_t mode)
{
    // geninput only uses 1 switch per panel, so ignore alternate modes
    return true;
}

bool bt_io_jb_coin_blocker_set(bool blocked)
{
    return true;
}

void bt_io_jb_rgb_led_set(
    bt_io_jb_rgb_led_t unit, uint8_t r, uint8_t g, uint8_t b)
{
    bt_input_mapper_light_write(unit * 3, r);
    bt_input_mapper_light_write(unit * 3 + 1, g);
    bt_input_mapper_light_write(unit * 3 + 2, b);
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_jb_api_get(bt_io_jb_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_jb_init;
    api->v1.fini = bt_io_jb_fini;
    api->v1.inputs_read = bt_io_jb_inputs_read;
    api->v1.sys_inputs_get = bt_io_jb_sys_inputs_get;
    api->v1.panel_inputs_get = bt_io_jb_panel_inputs_get;
    api->v1.rgb_led_set = bt_io_jb_rgb_led_set;
    api->v1.lights_write = bt_io_jb_lights_write;
    api->v1.panel_mode_set = bt_io_jb_panel_mode_set;
    api->v1.coin_blocker_set = bt_io_jb_coin_blocker_set;
}
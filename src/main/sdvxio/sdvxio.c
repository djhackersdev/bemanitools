#define LOG_MODULE "sdvxio"

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
#include "sdk/module/io/sdvx.h"

static uint16_t sdvx_io_gpio[2];
static uint8_t sdvx_io_gpio_sys;

static module_input_t *_sdvx_io_module_input;

static void _bt_io_sdvx_module_input_init(module_input_t **module)
{
    bt_input_api_t api;

    module_input_ext_load_and_init("geninput.dll", module);
    module_input_api_get(*module, &api);
    bt_input_api_set(&api);
}

bool bt_io_sdvx_init()
{
    bool result;

    _bt_io_sdvx_module_input_init(&_sdvx_io_module_input);

    timeBeginPeriod(1);

    result = bt_input_init();

    if (!result) {
        log_warning("Initializing input failed");
        return false;
    }

    return bt_input_mapper_config_load("sdvx");
}

void bt_io_sdvx_fini(void)
{
    bt_input_fini();
    bt_input_api_clear();
    module_input_free(&_sdvx_io_module_input);

    timeEndPeriod(1);
}

void bt_io_sdvx_gpio_lights_set(uint32_t gpio_lights)
{
    size_t i;

    for (i = 0; i < 16; i++) {
        if (gpio_lights & (1 << i)) {
            bt_input_mapper_light_write(i, 255);
        } else {
            bt_input_mapper_light_write(i, 0);
        }
    }
}

void bt_io_sdvx_pwm_light_set(uint8_t light_no, uint8_t intensity)
{
    bt_input_mapper_light_write(light_no + 0x10, intensity);
}

bool bt_io_sdvx_output_write(void)
{
    return true;
}

bool bt_io_sdvx_input_read(void)
{
    uint32_t pack;

    pack = bt_input_mapper_update();

    sdvx_io_gpio_sys = pack & 0xFF;
    sdvx_io_gpio[0] = (pack >> 8) & 0x00FF;
    sdvx_io_gpio[1] = (pack >> 16) & 0x00FF;

    return true;
}

uint8_t bt_io_sdvx_input_gpio_sys_get(void)
{
    return sdvx_io_gpio_sys;
}

uint16_t bt_io_sdvx_input_gpio_get(uint8_t gpio_bank)
{
    if (gpio_bank > 1) {
        return 0;
    }

    return sdvx_io_gpio[gpio_bank];
}

uint16_t bt_io_sdvx_spinner_pos_get(uint8_t spinner_no)
{
    return bt_input_mapper_analog_read(spinner_no) * 4;
}

bool bt_io_sdvx_amp_volume_set(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer)
{
    return true;
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_sdvx_api_get(bt_io_sdvx_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_sdvx_init;
    api->v1.fini = bt_io_sdvx_fini;
    api->v1.gpio_lights_set = bt_io_sdvx_gpio_lights_set;
    api->v1.pwm_light_set = bt_io_sdvx_pwm_light_set;
    api->v1.output_write = bt_io_sdvx_output_write;
    api->v1.input_read = bt_io_sdvx_input_read;
    api->v1.input_gpio_sys_get = bt_io_sdvx_input_gpio_sys_get;
    api->v1.input_gpio_get = bt_io_sdvx_input_gpio_get;
    api->v1.spinner_pos_get = bt_io_sdvx_spinner_pos_get;
    api->v1.amp_volume_set = bt_io_sdvx_amp_volume_set;
}

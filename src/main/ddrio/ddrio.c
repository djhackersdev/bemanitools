#define LOG_MODULE "ddrio"

#include <windows.h>

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
#include "sdk/module/io/ddr.h"

static module_input_t *_ddr_io_module_input;

bool bt_io_ddr_init()
{
    bool result;
    bt_input_api_t input_api;

    module_input_ext_load_and_init("geninput.dll", &_ddr_io_module_input);
    module_input_api_get(_ddr_io_module_input, &input_api);
    bt_input_api_set(&input_api);

    result = bt_input_init();

    if (!result) {
        return false;
    }

    return bt_input_mapper_config_load("ddr");
}

void bt_io_ddr_fini()
{
    bt_input_fini();

    bt_input_api_clear();
    module_input_free(&_ddr_io_module_input);
}

uint32_t bt_io_ddr_pad_read(void)
{
    /* Sleep first: input is timestamped immediately AFTER the ioctl returns.

       Which is the right thing to do, for once. We sleep here because
       the game polls input in a tight loop. Can't complain, at there isn't
       an artificial limit on the poll frequency. */

    Sleep(1);

    return (uint32_t) bt_input_mapper_update();
}

void bt_io_ddr_extio_lights_set(uint32_t lights)
{
    uint8_t i;

    for (i = 0x0E; i <= 0x1E; i++) {
        bt_input_mapper_light_write(i, lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_ddr_p3io_lights_set(uint32_t lights)
{
    uint8_t i;

    for (i = 0x00; i <= 0x07; i++) {
        bt_input_mapper_light_write(i, lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_ddr_hdxs_lights_panel_set(uint32_t lights)
{
    uint8_t i;

    for (i = 0x08; i <= 0x0D; i++) {
        bt_input_mapper_light_write(i, lights & (1 << i) ? 255 : 0);
    }
}

void bt_io_ddr_hdxs_lights_rgb_set(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx < 4) {
        uint8_t base = 0x20 + idx * 3;
        bt_input_mapper_light_write(base + 0, r);
        bt_input_mapper_light_write(base + 1, g);
        bt_input_mapper_light_write(base + 2, b);
    }
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_ddr_api_get(bt_io_ddr_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_ddr_init;
    api->v1.fini = bt_io_ddr_fini;
    api->v1.pad_read = bt_io_ddr_pad_read;
    api->v1.extio_lights_set = bt_io_ddr_extio_lights_set;
    api->v1.p3io_lights_set = bt_io_ddr_p3io_lights_set;
    api->v1.hdxs_lights_panel_set = bt_io_ddr_hdxs_lights_panel_set;
    api->v1.hdxs_lights_rgb_set = bt_io_ddr_hdxs_lights_rgb_set;
}
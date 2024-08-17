#define LOG_MODULE "jbio-p4io"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <setupapi.h>
#include <stdlib.h>
// clang-format on

#include "api/core/log.h"

#include "iface-acio/mgr.h"
#include "iface-core/config.h"
#include "iface-core/log.h"

#include "jbio-p4io/config-h44b.h"
#include "jbio-p4io/h44b.h"

#include "main/module/acio-mgr-ext.h"

#include "p4iodrv/device.h"

#include "sdk/module/core/config.h"
#include "sdk/module/configure.h"
#include "sdk/module/core/log.h"
#include "sdk/module/io/jb.h"

static h44b_config_t _jbio_p4io_config_h44b;

static struct p4iodrv_ctx *p4io_ctx;
static uint16_t jb_io_panels;
static uint8_t jb_io_sys_buttons;

static bool lights_present;
static struct ac_io_h44b_output jb_io_lights;
static struct ac_io_h44b_output jb_io_new_lights;

static bool coin_blocked;

static module_acio_mgr_t *acio_mgr_module;

static void _bt_io_jb_module_acio_mgr_init(module_acio_mgr_t **module)
{
    bt_acio_mgr_api_t api;

    module_acio_mgr_ext_load_and_init("acio-mgr.dll", module);
    module_acio_mgr_api_get(*module, &api);
    bt_acio_mgr_api_set(&api);
}

bool bt_io_jb_init()
{
    _bt_io_jb_module_acio_mgr_init(&acio_mgr_module);

    p4io_ctx = p4iodrv_open();
    if (!p4io_ctx) {
        return false;
    }

    // some people use p4io just for acio_mgrs and have no lights. Soft fail
    // when h44b is not able to be connected instead of returning false
    lights_present = jb_io_h44b_init(_jbio_p4io_config_h44b.port, _jbio_p4io_config_h44b.baud);
    if (!lights_present) {
        log_warning("Could not connect to H44B, lights disabled");
    }

    return true;
}

void bt_io_jb_fini()
{
    p4iodrv_close(p4io_ctx);

    bt_acio_mgr_api_clear();
    module_acio_mgr_free(&acio_mgr_module);
}

static const uint32_t jb_io_panel_mappings[] = {
    (1 << 5),
    (1 << 1),
    (1 << 13),
    (1 << 9),
    (1 << 6),
    (1 << 2),
    (1 << 14),
    (1 << 10),
    (1 << 7),
    (1 << 3),
    (1 << 15),
    (1 << 11),
    (1 << 16),
    (1 << 4),
    (1 << 20),
    (1 << 12),
};

static const uint32_t jb_io_sys_button_mappings[] = {
    (1 << 28),
    (1 << 25),
};

bool bt_io_jb_inputs_read()
{
    uint32_t jamma[4];
    if (!p4iodrv_read_jamma(p4io_ctx, jamma)) {
        return false;
    }

    jb_io_panels = 0;
    jb_io_sys_buttons = 0;

    // panel is active low
    uint32_t panel_in = ~jamma[0];

    for (uint8_t i = 0; i < 16; i++) {
        if (panel_in & jb_io_panel_mappings[i]) {
            jb_io_panels |= 1 << i;
        }
    }

    // sys is active high
    for (uint8_t i = 0; i < 2; i++) {
        if (jamma[0] & jb_io_sys_button_mappings[i]) {
            jb_io_sys_buttons |= 1 << i;
        }
    }

    return true;
}

bool bt_io_jb_lights_write()
{
    if (lights_present &&
        memcmp(&jb_io_lights, &jb_io_new_lights, sizeof(jb_io_lights))) {
        memcpy(&jb_io_lights, &jb_io_new_lights, sizeof(jb_io_lights));
        return jb_io_h44b_write_lights(&jb_io_lights);
    }

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
    struct p4io_req_panel_mode panel_mode = {0};

    panel_mode.is_single = 1;

    switch (mode) {
        case BT_IO_JB_PANEL_MODE_ALL:
            panel_mode.is_single = 0;
            panel_mode.mode = 0;
            break;
        case BT_IO_JB_PANEL_MODE_TOP_LEFT:
            panel_mode.mode = 0x0001;
            break;
        case BT_IO_JB_PANEL_MODE_TOP_RIGHT:
            panel_mode.mode = 0x0000;
            break;
        case BT_IO_JB_PANEL_MODE_BOTTOM_LEFT:
            panel_mode.mode = 0x0101;
            break;
        case BT_IO_JB_PANEL_MODE_BOTTOM_RIGHT:
            panel_mode.mode = 0x0100;
            break;
    }

    return p4iodrv_cmd_portout(p4io_ctx, (uint8_t *) &panel_mode);
}

void bt_io_jb_rgb_led_set(
    bt_io_jb_rgb_led_t unit, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t *light;

    switch (unit) {
        case BT_IO_JB_RGB_LED_FRONT:
            light = jb_io_new_lights.front_rgb;
            break;
        case BT_IO_JB_RGB_LED_TOP:
            light = jb_io_new_lights.top_rgb;
            break;
        case BT_IO_JB_RGB_LED_LEFT:
            light = jb_io_new_lights.left_rgb;
            break;
        case BT_IO_JB_RGB_LED_RIGHT:
            light = jb_io_new_lights.right_rgb;
            break;
        case BT_IO_JB_RGB_LED_TITLE:
            light = jb_io_new_lights.title_rgb;
            break;
        case BT_IO_JB_RGB_LED_WOOFER:
            light = jb_io_new_lights.woofer_rgb;
            break;
        default:
            return;
    }

    light[0] = r;
    light[1] = g;
    light[2] = b;
}

bool bt_io_jb_coin_blocker_set(bool blocked)
{
    coin_blocked = blocked;

    uint8_t coin[4] = {0};
    coin[0] = coin_blocked ? 0x00 : 0x20;
    return p4iodrv_cmd_coinstock(p4io_ctx, coin);
}

void bt_module_core_config_api_set(const bt_core_config_api_t *api)
{
    bt_core_config_api_set(api);
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

bool bt_module_configure_do(const bt_core_config_t *config)
{
    jbio_config_h44b_get(config, &_jbio_p4io_config_h44b);

    return true;
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

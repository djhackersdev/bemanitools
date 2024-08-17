#define LOG_MODULE "jbio-magicbox"

// This implementation is ported from the device.dll source graciously
// provided by zyp

#include <windows.h>

#include "api/core/log.h"

#include "iface-core/log.h"

#include "imports/ch341.h"

#include "sdk/module/core/log.h"
#include "sdk/module/io/jb.h"

#include "util/defs.h"

static uint16_t jb_io_panels;
static uint8_t jb_io_sys_buttons;

static bool is_initialized = false;

union magicbox_input {
    uint32_t dword;
    uint8_t bytes[4];
};

bool bt_io_jb_init()
{
    if (CH341OpenDevice(0) < 0) {
        log_warning("jbio", "Can't open CH341 device.\n");
        return false;
    }

    is_initialized = true;

    return true;
}

void bt_io_jb_fini()
{
    CH341CloseDevice(0);
}

static const uint32_t magic_panel_mappings[] = {
    (1 << 0x08),
    (1 << 0x0a),
    (1 << 0x0c),
    (1 << 0x0e),
    (1 << 0x07),
    (1 << 0x05),
    (1 << 0x03),
    (1 << 0x01),
    (1 << 0x09),
    (1 << 0x0b),
    (1 << 0x0d),
    (1 << 0x0f),
    (1 << 0x06),
    (1 << 0x04),
    (1 << 0x02),
    (1 << 0x00),
};

static const uint32_t magic_sys_mappings[] = {
    (1 << 0x11), // TEST
    (1 << 0x10), // SERVICE
    (1 << 0x13), // COIN
};

bool bt_io_jb_inputs_read()
{
    // Read IO board
    unsigned long size;
    union magicbox_input input;

    input.dword = -1;
    jb_io_panels = 0;
    jb_io_sys_buttons = 0;

    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 1);
    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 0);
    size = sizeof(uint8_t);
    CH341EppReadData(0, &input.bytes[0], &size);
    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 0);
    size = sizeof(uint8_t);
    CH341EppReadData(0, &input.bytes[1], &size);
    CH341EppSetAddr(0, 255);
    CH341EppSetAddr(0, 0);
    size = sizeof(uint8_t);
    CH341EppReadData(0, &input.bytes[2], &size);
    CH341EppSetAddr(0, 255);

    for (uint8_t i = 0; i < lengthof(magic_panel_mappings); i++) {
        if ((input.dword & magic_panel_mappings[i]) == 0) {
            jb_io_panels |= 1 << i;
        }
    }
    for (uint8_t i = 0; i < lengthof(magic_sys_mappings); i++) {
        if ((input.dword & magic_sys_mappings[i]) == 0) {
            jb_io_sys_buttons |= 1 << i;
        }
    }

    return true;
}

bool bt_io_jb_lights_write()
{
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

void bt_io_jb_rgb_led_set(
    bt_io_jb_rgb_led_t unit, uint8_t r, uint8_t g, uint8_t b)
{
    // I mean I guess there's reactive LEDs on the sides? I'm not going to the
    // effort to work out if they're controllable or not
}

bool bt_io_jb_panel_mode_set(bt_io_jb_panel_mode_t mode)
{
    // panel always returns merged input state, no corner support
    return true;
}

bool bt_io_jb_coin_blocker_set(bool blocked)
{
    return true;
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
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
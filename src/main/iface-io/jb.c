#define LOG_MODULE "bt-io-jb"

#include <string.h>

#include "api/io/jb.h"

#include "iface-core/log.h"

#define BT_IO_JB_ASSERT_IMPLEMENTED(func, name)                    \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_jb_api_t _bt_io_jb_api;

static bool _bt_io_jb_api_is_valid()
{
    return _bt_io_jb_api.version > 0;
}

void bt_io_jb_api_set(const bt_io_jb_api_t *api)
{
    log_assert(api);

    if (_bt_io_jb_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_JB_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_jb_init");
        BT_IO_JB_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_jb_fini");

        BT_IO_JB_ASSERT_IMPLEMENTED(
            api->v1.inputs_read, "bt_io_jb_inputs_read");
        BT_IO_JB_ASSERT_IMPLEMENTED(
            api->v1.sys_inputs_get, "bt_io_jb_sys_inputs_get");
        BT_IO_JB_ASSERT_IMPLEMENTED(
            api->v1.panel_inputs_get, "bt_io_jb_panel_inputs_get");
        BT_IO_JB_ASSERT_IMPLEMENTED(
            api->v1.rgb_led_set, "bt_io_jb_rgb_led_set");
        BT_IO_JB_ASSERT_IMPLEMENTED(
            api->v1.lights_write, "bt_io_jb_lights_write");
        BT_IO_JB_ASSERT_IMPLEMENTED(
            api->v1.panel_mode_set, "bt_io_jb_panel_mode_set");
        BT_IO_JB_ASSERT_IMPLEMENTED(
            api->v1.coin_blocker_set, "bt_io_jb_coin_blocker_set");

        memcpy(&_bt_io_jb_api, api, sizeof(bt_io_jb_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_jb_api_get(bt_io_jb_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_jb_api_is_valid());

    memcpy(api, &_bt_io_jb_api, sizeof(bt_io_jb_api_t));
}

void bt_io_jb_api_clear()
{
    log_assert(_bt_io_jb_api_is_valid());

    memset(&_bt_io_jb_api, 0, sizeof(bt_io_jb_api_t));

    log_misc("api cleared");
}

bool bt_io_jb_init()
{
    bool result;

    log_assert(_bt_io_jb_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_jb_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_jb_fini()
{
    log_assert(_bt_io_jb_api_is_valid());

    log_misc(">>> fini");

    _bt_io_jb_api.v1.fini();

    log_misc("<<< fini");
}

bool bt_io_jb_inputs_read()
{
    log_assert(_bt_io_jb_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_jb_api.v1.inputs_read();
}

uint8_t bt_io_jb_sys_inputs_get()
{
    log_assert(_bt_io_jb_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_jb_api.v1.sys_inputs_get();
}

uint16_t bt_io_jb_panel_inputs_get()
{
    log_assert(_bt_io_jb_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_jb_api.v1.panel_inputs_get();
}

void bt_io_jb_rgb_led_set(
    bt_io_jb_rgb_led_t unit, uint8_t r, uint8_t g, uint8_t b)
{
    log_assert(_bt_io_jb_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_jb_api.v1.rgb_led_set(unit, r, g, b);
}

bool bt_io_jb_lights_write()
{
    log_assert(_bt_io_jb_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_jb_api.v1.lights_write();
}

bool bt_io_jb_panel_mode_set(bt_io_jb_panel_mode_t mode)
{
    log_assert(_bt_io_jb_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_jb_api.v1.panel_mode_set(mode);
}

bool bt_io_jb_coin_blocker_set(bool blocked)
{
    log_assert(_bt_io_jb_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_jb_api.v1.coin_blocker_set(blocked);
}

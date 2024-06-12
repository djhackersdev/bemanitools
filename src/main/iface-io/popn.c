#define LOG_MODULE "bt-io-popn"

#include <string.h>

#include "api/io/popn.h"

#include "iface-core/log.h"

#define BT_IO_POPN_ASSERT_IMPLEMENTED(func, name)                  \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_popn_api_t _bt_io_popn_api;

static bool _bt_io_popn_api_is_valid()
{
    return _bt_io_popn_api.version > 0;
}

void bt_io_popn_api_set(const bt_io_popn_api_t *api)
{
    log_assert(api);

    if (_bt_io_popn_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_POPN_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_popn_init");
        BT_IO_POPN_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_popn_fini");

        BT_IO_POPN_ASSERT_IMPLEMENTED(
            api->v1.buttons_get, "bt_io_popn_buttons_get");
        BT_IO_POPN_ASSERT_IMPLEMENTED(
            api->v1.top_lights_set, "bt_io_popn_top_lights_set");
        BT_IO_POPN_ASSERT_IMPLEMENTED(
            api->v1.side_lights_set, "bt_io_popn_side_lights_set");
        BT_IO_POPN_ASSERT_IMPLEMENTED(
            api->v1.button_lights_set, "bt_io_popn_button_lights_set");
        BT_IO_POPN_ASSERT_IMPLEMENTED(
            api->v1.coin_blocker_light_set,
            "bt_io_popn_coin_blocker_light_set");
        BT_IO_POPN_ASSERT_IMPLEMENTED(
            api->v1.coin_counter_light_set,
            "bt_io_popn_coin_counter_light_set");

        memcpy(&_bt_io_popn_api, api, sizeof(bt_io_popn_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_popn_api_get(bt_io_popn_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_popn_api_is_valid());

    memcpy(api, &_bt_io_popn_api, sizeof(bt_io_popn_api_t));
}

void bt_io_popn_api_clear()
{
    log_assert(_bt_io_popn_api_is_valid());

    memset(&_bt_io_popn_api, 0, sizeof(bt_io_popn_api_t));

    log_misc("api cleared");
}

bool bt_io_popn_init()
{
    bool result;

    log_assert(_bt_io_popn_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_popn_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_popn_fini()
{
    log_assert(_bt_io_popn_api_is_valid());

    log_misc(">>> fini");

    _bt_io_popn_api.v1.fini();

    log_misc("<<< fini");
}

uint32_t bt_io_popn_buttons_get()
{
    log_assert(_bt_io_popn_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_popn_api.v1.buttons_get();
}

void bt_io_popn_top_lights_set(uint32_t lights)
{
    log_assert(_bt_io_popn_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_popn_api.v1.top_lights_set(lights);
}

void bt_io_popn_side_lights_set(uint32_t lights)
{
    log_assert(_bt_io_popn_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_popn_api.v1.side_lights_set(lights);
}

void bt_io_popn_button_lights_set(uint32_t lights)
{
    log_assert(_bt_io_popn_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_popn_api.v1.button_lights_set(lights);
}

void bt_io_popn_coin_blocker_light_set(bool enabled)
{
    log_assert(_bt_io_popn_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_popn_api.v1.coin_blocker_light_set(enabled);
}

void bt_io_popn_coin_counter_light_set(bool enabled)
{
    log_assert(_bt_io_popn_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_popn_api.v1.coin_counter_light_set(enabled);
}

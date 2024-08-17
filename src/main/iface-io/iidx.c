#define LOG_MODULE "bt-io-iidx"

#include <string.h>

#include "api/io/iidx.h"

#include "iface-core/log.h"

#define BT_IO_IIDX_ASSERT_IMPLEMENTED(func, name)                  \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_iidx_api_t _bt_io_iidx_api;

static bool _bt_io_iidx_api_is_valid()
{
    return _bt_io_iidx_api.version > 0;
}

void bt_io_iidx_api_set(const bt_io_iidx_api_t *api)
{
    log_assert(api);

    if (_bt_io_iidx_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_IIDX_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_iidx_init");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_iidx_fini");

        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep1_deck_lights_set, "bt_io_iidx_ep1_deck_lights_set");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep1_panel_lights_set, "bt_io_iidx_ep1_panel_lights_set");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep1_top_lamps_set, "bt_io_iidx_ep1_top_lamps_set");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep1_top_neons_set, "bt_io_iidx_ep1_top_neons_set");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(api->v1.ep1_send, "bt_io_iidx_ep1_send");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(api->v1.ep2_recv, "bt_io_iidx_ep2_recv");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep2_turntable_get, "bt_io_iidx_ep2_turntable_get");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep2_slider_get, "bt_io_iidx_ep2_slider_get");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep2_sys_get, "bt_io_iidx_ep2_sys_get");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep2_panel_get, "bt_io_iidx_ep2_panel_get");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep2_keys_get, "bt_io_iidx_ep2_keys_get");
        BT_IO_IIDX_ASSERT_IMPLEMENTED(
            api->v1.ep3_16seg_send, "bt_io_iidx_ep3_16seg_send");

        memcpy(&_bt_io_iidx_api, api, sizeof(bt_io_iidx_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_iidx_api_get(bt_io_iidx_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_iidx_api_is_valid());

    memcpy(api, &_bt_io_iidx_api, sizeof(bt_io_iidx_api_t));
}

void bt_io_iidx_api_clear()
{
    log_assert(_bt_io_iidx_api_is_valid());

    memset(&_bt_io_iidx_api, 0, sizeof(bt_io_iidx_api_t));

    log_misc("api cleared");
}

bool bt_io_iidx_init()
{
    bool result;

    log_assert(_bt_io_iidx_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_iidx_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_iidx_fini()
{
    log_assert(_bt_io_iidx_api_is_valid());

    log_misc(">>> fini");

    _bt_io_iidx_api.v1.fini();

    log_misc("<<< fini");
}

void bt_io_iidx_ep1_deck_lights_set(uint16_t deck_lights)
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_iidx_api.v1.ep1_deck_lights_set(deck_lights);
}

void bt_io_iidx_ep1_panel_lights_set(uint8_t panel_lights)
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_iidx_api.v1.ep1_panel_lights_set(panel_lights);
}

void bt_io_iidx_ep1_top_lamps_set(uint8_t top_lamps)
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_iidx_api.v1.ep1_top_lamps_set(top_lamps);
}

void bt_io_iidx_ep1_top_neons_set(bool top_neons)
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_iidx_api.v1.ep1_top_neons_set(top_neons);
}

bool bt_io_iidx_ep1_send()
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep1_send();
}

bool bt_io_iidx_ep2_recv()
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep2_recv();
}

uint8_t bt_io_iidx_ep2_turntable_get(uint8_t player_no)
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep2_turntable_get(player_no);
}

uint8_t bt_io_iidx_ep2_slider_get(uint8_t slider_no)
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep2_slider_get(slider_no);
}

uint8_t bt_io_iidx_ep2_sys_get()
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep2_sys_get();
}

uint8_t bt_io_iidx_ep2_panel_get()
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep2_panel_get();
}

uint16_t bt_io_iidx_ep2_keys_get()
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep2_keys_get();
}

bool bt_io_iidx_ep3_16seg_send(const char *text)
{
    log_assert(_bt_io_iidx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_iidx_api.v1.ep3_16seg_send(text);
}

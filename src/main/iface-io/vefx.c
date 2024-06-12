#define LOG_MODULE "bt-io-vefx"

#include <string.h>

#include "api/io/vefx.h"

#include "iface-core/log.h"

#define BT_IO_VEFX_ASSERT_IMPLEMENTED(func, name)                  \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_vefx_api_t _bt_io_vefx_api;

static bool _bt_io_vefx_api_is_valid()
{
    return _bt_io_vefx_api.version > 0;
}

void bt_io_vefx_api_set(const bt_io_vefx_api_t *api)
{
    log_assert(api);

    if (_bt_io_vefx_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_VEFX_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_vefx_init");
        BT_IO_VEFX_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_vefx_fini");

        BT_IO_VEFX_ASSERT_IMPLEMENTED(api->v1.recv, "bt_io_vefx_recv");
        BT_IO_VEFX_ASSERT_IMPLEMENTED(
            api->v1.slider_get, "bt_io_vefx_slider_get");
        BT_IO_VEFX_ASSERT_IMPLEMENTED(
            api->v1._16seg_send, "bt_io_vefx_16seg_send");

        memcpy(&_bt_io_vefx_api, api, sizeof(bt_io_vefx_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_vefx_api_get(bt_io_vefx_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_vefx_api_is_valid());

    memcpy(api, &_bt_io_vefx_api, sizeof(bt_io_vefx_api_t));
}

void bt_io_vefx_api_clear()
{
    log_assert(_bt_io_vefx_api_is_valid());

    memset(&_bt_io_vefx_api, 0, sizeof(bt_io_vefx_api_t));

    log_misc("api cleared");
}

bool bt_io_vefx_init()
{
    bool result;

    log_assert(_bt_io_vefx_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_vefx_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_vefx_fini()
{
    log_assert(_bt_io_vefx_api_is_valid());

    log_misc(">>> fini");

    _bt_io_vefx_api.v1.fini();

    log_misc("<<< fini");
}

bool bt_io_vefx_recv(uint64_t *ppad)
{
    log_assert(_bt_io_vefx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_vefx_api.v1.recv(ppad);
}

uint8_t bt_io_vefx_slider_get(uint8_t slider_no)
{
    log_assert(_bt_io_vefx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_vefx_api.v1.slider_get(slider_no);
}

bool bt_io_vefx_16seg_send(const char *text)
{
    log_assert(_bt_io_vefx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_vefx_api.v1._16seg_send(text);
}

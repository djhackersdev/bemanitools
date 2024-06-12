#define LOG_MODULE "bt-io-ddr"

#include <string.h>

#include "api/io/ddr.h"

#include "iface-core/log.h"

#define BT_IO_DDR_ASSERT_IMPLEMENTED(func, name)                   \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_ddr_api_t _bt_io_ddr_api;

static bool _bt_io_ddr_api_is_valid()
{
    return _bt_io_ddr_api.version > 0;
}

void bt_io_ddr_api_set(const bt_io_ddr_api_t *api)
{
    log_assert(api);

    if (_bt_io_ddr_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_DDR_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_ddr_init");
        BT_IO_DDR_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_ddr_fini");

        BT_IO_DDR_ASSERT_IMPLEMENTED(api->v1.pad_read, "bt_io_ddr_pad_read");
        BT_IO_DDR_ASSERT_IMPLEMENTED(
            api->v1.extio_lights_set, "bt_io_ddr_extio_lights_set");
        BT_IO_DDR_ASSERT_IMPLEMENTED(
            api->v1.p3io_lights_set, "bt_io_ddr_p3io_lights_set");
        BT_IO_DDR_ASSERT_IMPLEMENTED(
            api->v1.hdxs_lights_panel_set, "bt_io_ddr_hdxs_lights_panel_set");
        BT_IO_DDR_ASSERT_IMPLEMENTED(
            api->v1.hdxs_lights_rgb_set, "bt_io_ddr_hdxs_lights_rgb_set");

        memcpy(&_bt_io_ddr_api, api, sizeof(bt_io_ddr_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_ddr_api_get(bt_io_ddr_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_ddr_api_is_valid());

    memcpy(api, &_bt_io_ddr_api, sizeof(bt_io_ddr_api_t));
}

void bt_io_ddr_api_clear()
{
    log_assert(_bt_io_ddr_api_is_valid());

    memset(&_bt_io_ddr_api, 0, sizeof(bt_io_ddr_api_t));

    log_misc("api cleared");
}

bool bt_io_ddr_init()
{
    bool result;

    log_assert(_bt_io_ddr_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_ddr_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_ddr_fini()
{
    log_assert(_bt_io_ddr_api_is_valid());

    log_misc(">>> fini");

    _bt_io_ddr_api.v1.fini();

    log_misc("<<< fini");
}

uint32_t bt_io_ddr_pad_read()
{
    log_assert(_bt_io_ddr_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_ddr_api.v1.pad_read();
}

void bt_io_ddr_extio_lights_set(uint32_t extio_lights)
{
    log_assert(_bt_io_ddr_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_ddr_api.v1.extio_lights_set(extio_lights);
}

void bt_io_ddr_p3io_lights_set(uint32_t p3io_lights)
{
    log_assert(_bt_io_ddr_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_ddr_api.v1.p3io_lights_set(p3io_lights);
}

void bt_io_ddr_hdxs_lights_panel_set(uint32_t hdxs_lights)
{
    log_assert(_bt_io_ddr_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_ddr_api.v1.hdxs_lights_panel_set(hdxs_lights);
}

void bt_io_ddr_hdxs_lights_rgb_set(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    log_assert(_bt_io_ddr_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_ddr_api.v1.hdxs_lights_rgb_set(idx, r, g, b);
}

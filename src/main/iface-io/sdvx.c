#define LOG_MODULE "bt-io-sdvx"

#include <string.h>

#include "api/io/sdvx.h"

#include "iface-core/log.h"

#define BT_IO_SDVX_ASSERT_IMPLEMENTED(func, name)                  \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_sdvx_api_t _bt_io_sdvx_api;

static bool _bt_io_sdvx_api_is_valid()
{
    return _bt_io_sdvx_api.version > 0;
}

void bt_io_sdvx_api_set(const bt_io_sdvx_api_t *api)
{
    log_assert(api);

    if (_bt_io_sdvx_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_SDVX_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_sdvx_init");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_sdvx_fini");

        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.gpio_lights_set, "bt_io_sdvx_gpio_lights_set");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.pwm_light_set, "bt_io_sdvx_pwm_light_set");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.output_write, "bt_io_sdvx_output_write");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.input_read, "bt_io_sdvx_input_read");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.input_gpio_sys_get, "bt_io_sdvx_input_gpio_sys_get");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.input_gpio_get, "bt_io_sdvx_input_gpio_get");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.spinner_pos_get, "bt_io_sdvx_spinner_pos_get");
        BT_IO_SDVX_ASSERT_IMPLEMENTED(
            api->v1.amp_volume_set, "bt_io_sdvx_amp_volume_set");

        memcpy(&_bt_io_sdvx_api, api, sizeof(bt_io_sdvx_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_sdvx_api_get(bt_io_sdvx_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_sdvx_api_is_valid());

    memcpy(api, &_bt_io_sdvx_api, sizeof(bt_io_sdvx_api_t));
}

void bt_io_sdvx_api_clear()
{
    log_assert(_bt_io_sdvx_api_is_valid());

    memset(&_bt_io_sdvx_api, 0, sizeof(bt_io_sdvx_api_t));

    log_misc("api cleared");
}

bool bt_io_sdvx_init()
{
    bool result;

    log_assert(_bt_io_sdvx_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_sdvx_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_sdvx_fini()
{
    log_assert(_bt_io_sdvx_api_is_valid());

    log_misc(">>> fini");

    _bt_io_sdvx_api.v1.fini();

    log_misc("<<< fini");
}

void bt_io_sdvx_gpio_lights_set(uint32_t gpio_lights)
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_sdvx_api.v1.gpio_lights_set(gpio_lights);
}

void bt_io_sdvx_pwm_light_set(uint8_t light_no, uint8_t intensity)
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    _bt_io_sdvx_api.v1.pwm_light_set(light_no, intensity);
}

bool bt_io_sdvx_output_write()
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_sdvx_api.v1.output_write();
}

bool bt_io_sdvx_input_read()
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_sdvx_api.v1.input_read();
}

uint8_t bt_io_sdvx_input_gpio_sys_get()
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_sdvx_api.v1.input_gpio_sys_get();
}

uint16_t bt_io_sdvx_input_gpio_get(uint8_t gpio_bank)
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_sdvx_api.v1.input_gpio_get(gpio_bank);
}

uint16_t bt_io_sdvx_spinner_pos_get(uint8_t spinner_no)
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_sdvx_api.v1.spinner_pos_get(spinner_no);
}

bool bt_io_sdvx_amp_volume_set(
    uint8_t primary, uint8_t headphone, uint8_t subwoofer)
{
    log_assert(_bt_io_sdvx_api_is_valid());

    // Do not log on frequently invoked calls to avoid negative performance
    // impact and log spam

    return _bt_io_sdvx_api.v1.amp_volume_set(primary, headphone, subwoofer);
}

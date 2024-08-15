#define LOG_MODULE "bt-input"

#include <string.h>

#include "api/input.h"

#include "iface-core/log.h"

#define BT_INPUT_ASSERT_IMPLEMENTED(func, name)                    \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_input_api_t _bt_input_api;

static bool _bt_input_api_is_valid()
{
    return _bt_input_api.version > 0;
}

void bt_input_api_set(const bt_input_api_t *api)
{
    log_assert(api);

    if (_bt_input_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_INPUT_ASSERT_IMPLEMENTED(api->v1.init, "bt_input_init");
        BT_INPUT_ASSERT_IMPLEMENTED(api->v1.fini, "bt_input_fini");

        BT_INPUT_ASSERT_IMPLEMENTED(
            api->v1.mapper_config_load, "bt_input_mapper_config_load");
        BT_INPUT_ASSERT_IMPLEMENTED(
            api->v1.mapper_analog_read, "bt_input_mapper_analog_read");
        BT_INPUT_ASSERT_IMPLEMENTED(
            api->v1.mapper_update, "bt_input_mapper_update");
        BT_INPUT_ASSERT_IMPLEMENTED(
            api->v1.mapper_light_write, "bt_input_mapper_light_write");

        memcpy(&_bt_input_api, api, sizeof(bt_input_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_input_api_get(bt_input_api_t *api)
{
    log_assert(api);
    log_assert(_bt_input_api_is_valid());

    memcpy(api, &_bt_input_api, sizeof(bt_input_api_t));
}

void bt_input_api_clear()
{
    log_assert(_bt_input_api_is_valid());

    memset(&_bt_input_api, 0, sizeof(bt_input_api_t));

    log_misc("api cleared");
}

bool bt_input_init()
{
    bool result;

    log_assert(_bt_input_api_is_valid());

    log_misc(">>> init");

    result = _bt_input_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_input_fini()
{
    log_assert(_bt_input_api_is_valid());

    log_misc(">>> fini");

    _bt_input_api.v1.fini();

    log_misc("<<< fini");
}

bool bt_input_mapper_config_load(const char *game_type)
{
    bool result;

    log_assert(_bt_input_api_is_valid());

    log_misc(">>> mapper_config_load: %s", game_type);

    result = _bt_input_api.v1.mapper_config_load(game_type);

    log_misc("<<< mapper_config_load: %d", result);

    return result;
}

uint8_t bt_input_mapper_analog_read(uint8_t analog)
{
    log_assert(_bt_input_api_is_valid());

    return _bt_input_api.v1.mapper_analog_read(analog);
}

uint64_t bt_input_mapper_update()
{
    log_assert(_bt_input_api_is_valid());

    return _bt_input_api.v1.mapper_update();
}

void bt_input_mapper_light_write(uint8_t light, uint8_t intensity)
{
    log_assert(_bt_input_api_is_valid());

    return _bt_input_api.v1.mapper_light_write(light, intensity);
}

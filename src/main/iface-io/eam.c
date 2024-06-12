#define LOG_MODULE "bt-io-eam"

#include <string.h>

#include "api/io/eam.h"

#include "iface-core/log.h"

#define BT_IO_EAM_ASSERT_IMPLEMENTED(func, name)                   \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_eam_api_t _bt_io_eam_api;

static bool _bt_io_eam_api_is_valid()
{
    return _bt_io_eam_api.version > 0;
}

void bt_io_eam_api_set(const bt_io_eam_api_t *api)
{
    log_assert(api);

    if (_bt_io_eam_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_EAM_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_eam_init");
        BT_IO_EAM_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_eam_fini");

        BT_IO_EAM_ASSERT_IMPLEMENTED(
            api->v1.keypad_state_get, "bt_io_eam_keypad_state_get");
        BT_IO_EAM_ASSERT_IMPLEMENTED(
            api->v1.sensor_state_get, "bt_io_eam_sensor_state_get");
        BT_IO_EAM_ASSERT_IMPLEMENTED(api->v1.card_read, "bt_io_eam_card_read");
        BT_IO_EAM_ASSERT_IMPLEMENTED(
            api->v1.card_slot_cmd_send, "bt_io_eam_card_slot_cmd_send");
        BT_IO_EAM_ASSERT_IMPLEMENTED(api->v1.poll, "bt_io_eam_poll");

        BT_IO_EAM_ASSERT_IMPLEMENTED(
            api->v1.config_api_get, "bt_io_eam_config_api_get");

        memcpy(&_bt_io_eam_api, api, sizeof(bt_io_eam_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_eam_api_get(bt_io_eam_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_eam_api_is_valid());

    memcpy(api, &_bt_io_eam_api, sizeof(bt_io_eam_api_t));
}

void bt_io_eam_api_clear()
{
    log_assert(_bt_io_eam_api_is_valid());

    memset(&_bt_io_eam_api, 0, sizeof(bt_io_eam_api_t));

    log_misc("api cleared");
}

bool bt_io_eam_init()
{
    bool result;

    log_assert(_bt_io_eam_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_eam_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_eam_fini()
{
    log_assert(_bt_io_eam_api_is_valid());

    log_misc(">>> fini");

    _bt_io_eam_api.v1.fini();

    log_misc("<<< fini");
}

uint16_t bt_io_eam_keypad_state_get(uint8_t unit_no)
{
    log_assert(_bt_io_eam_api_is_valid());

    return _bt_io_eam_api.v1.keypad_state_get(unit_no);
}

uint8_t bt_io_eam_sensor_state_get(uint8_t unit_no)
{
    log_assert(_bt_io_eam_api_is_valid());

    return _bt_io_eam_api.v1.sensor_state_get(unit_no);
}

uint8_t bt_io_eam_card_read(uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    log_assert(_bt_io_eam_api_is_valid());
    log_assert(card_id);

    return _bt_io_eam_api.v1.card_read(unit_no, card_id, nbytes);
}

bool bt_io_eam_card_slot_cmd_send(uint8_t unit_no, uint8_t cmd)
{
    log_assert(_bt_io_eam_api_is_valid());

    return _bt_io_eam_api.v1.card_slot_cmd_send(unit_no, cmd);
}

bool bt_io_eam_poll(uint8_t unit_no)
{
    log_assert(_bt_io_eam_api_is_valid());

    return _bt_io_eam_api.v1.poll(unit_no);
}

const bt_io_eam_config_api_t *bt_io_eam_config_api_get()
{
    log_assert(_bt_io_eam_api_is_valid());

    return _bt_io_eam_api.v1.config_api_get();
}
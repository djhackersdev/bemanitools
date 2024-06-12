#define LOG_MODULE "bt-io-bst"

#include <string.h>

#include "api/io/bst.h"

#include "iface-core/log.h"

#define BT_IO_BST_ASSERT_IMPLEMENTED(func, name)                   \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_io_bst_api_t _bt_io_bst_api;

static bool _bt_io_bst_api_is_valid()
{
    return _bt_io_bst_api.version > 0;
}

void bt_io_bst_api_set(const bt_io_bst_api_t *api)
{
    log_assert(api);

    if (_bt_io_bst_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_IO_BST_ASSERT_IMPLEMENTED(api->v1.init, "bt_io_bst_init");
        BT_IO_BST_ASSERT_IMPLEMENTED(api->v1.fini, "bt_io_bst_fini");

        BT_IO_BST_ASSERT_IMPLEMENTED(
            api->v1.input_read, "bt_io_bst_input_read");
        BT_IO_BST_ASSERT_IMPLEMENTED(api->v1.input_get, "bt_io_bst_input_get");

        memcpy(&_bt_io_bst_api, api, sizeof(bt_io_bst_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_io_bst_api_get(bt_io_bst_api_t *api)
{
    log_assert(api);
    log_assert(_bt_io_bst_api_is_valid());

    memcpy(api, &_bt_io_bst_api, sizeof(bt_io_bst_api_t));
}

void bt_io_bst_api_clear()
{
    log_assert(_bt_io_bst_api_is_valid());

    memset(&_bt_io_bst_api, 0, sizeof(bt_io_bst_api_t));

    log_misc("api cleared");
}

bool bt_io_bst_init()
{
    bool result;

    log_assert(_bt_io_bst_api_is_valid());

    log_misc(">>> init");

    result = _bt_io_bst_api.v1.init();

    log_misc("<<< init: %d", result);

    return result;
}

void bt_io_bst_fini()
{
    log_assert(_bt_io_bst_api_is_valid());

    log_misc(">>> fini");

    _bt_io_bst_api.v1.fini();

    log_misc("<<< fini");
}

bool bt_io_bst_input_read()
{
    log_assert(_bt_io_bst_api_is_valid());

    return _bt_io_bst_api.v1.input_read();
}

uint8_t bt_io_bst_input_get()
{
    log_assert(_bt_io_bst_api_is_valid());

    return _bt_io_bst_api.v1.input_get();
}
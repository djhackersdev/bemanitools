#define LOG_MODULE "bt-acio-mgr"

#include <string.h>

#include "api/acio/mgr.h"

#include "iface-core/log.h"

#define BT_ACIO_MGR_ASSERT_IMPLEMENTED(func, name)                 \
    if (!func) {                                                   \
        log_fatal("Function %s not implemented", STRINGIFY(name)); \
    }

static bt_acio_mgr_api_t _bt_acio_mgr_api;

static bool _bt_acio_mgr_api_is_valid()
{
    return _bt_acio_mgr_api.version > 0;
}

void bt_acio_mgr_api_set(const bt_acio_mgr_api_t *api)
{
    log_assert(api);

    if (_bt_acio_mgr_api_is_valid()) {
        log_warning("Re-initialize");
    }

    if (api->version == 1) {
        BT_ACIO_MGR_ASSERT_IMPLEMENTED(
            api->v1.port_init, "bt_acio_mgr_port_init");
        BT_ACIO_MGR_ASSERT_IMPLEMENTED(
            api->v1.port_fini, "bt_acio_mgr_port_fini");
        BT_ACIO_MGR_ASSERT_IMPLEMENTED(
            api->v1.node_count_get, "bt_acio_mgr_node_count_get");
        BT_ACIO_MGR_ASSERT_IMPLEMENTED(
            api->v1.node_product_ident_get,
            "bt_acio_mgr_node_product_ident_get");
        BT_ACIO_MGR_ASSERT_IMPLEMENTED(
            api->v1.port_packet_submit, "bt_acio_mgr_port_packet_submit");
        BT_ACIO_MGR_ASSERT_IMPLEMENTED(
            api->v1.port_checkout, "bt_acio_mgr_port_checkout");
        BT_ACIO_MGR_ASSERT_IMPLEMENTED(
            api->v1.port_checkin, "bt_acio_mgr_port_checkin");

        memcpy(&_bt_acio_mgr_api, api, sizeof(bt_acio_mgr_api_t));

        log_misc("api v1 set");
    } else {
        log_fatal("Unsupported API version: %d", api->version);
    }
}

void bt_acio_mgr_api_get(bt_acio_mgr_api_t *api)
{
    log_assert(api);
    log_assert(_bt_acio_mgr_api_is_valid());

    memcpy(api, &_bt_acio_mgr_api, sizeof(bt_acio_mgr_api_t));
}

void bt_acio_mgr_api_clear()
{
    log_assert(_bt_acio_mgr_api_is_valid());

    memset(&_bt_acio_mgr_api, 0, sizeof(bt_acio_mgr_api_t));

    log_misc("api cleared");
}

bt_acio_mgr_port_dispatcher_t *
bt_acio_mgr_port_init(const char *path, uint32_t baud)
{
    log_assert(_bt_acio_mgr_api_is_valid());

    log_misc("port init: %s, %d", path, baud);

    return _bt_acio_mgr_api.v1.port_init(path, baud);
}

void bt_acio_mgr_port_fini(bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    log_assert(_bt_acio_mgr_api_is_valid());

    log_misc(">>> port fini");

    _bt_acio_mgr_api.v1.port_fini(dispatcher);

    log_misc("<<< port fini");
}

uint8_t
bt_acio_mgr_node_count_get(const bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    log_assert(_bt_acio_mgr_api_is_valid());

    return _bt_acio_mgr_api.v1.node_count_get(dispatcher);
}

bool bt_acio_mgr_node_product_ident_get(
    const bt_acio_mgr_port_dispatcher_t *dispatcher,
    uint8_t node_id,
    char product[BT_ACIO_MGR_NODE_PRODUCT_CODE_LEN])
{
    log_assert(_bt_acio_mgr_api_is_valid());

    return _bt_acio_mgr_api.v1.node_product_ident_get(
        dispatcher, node_id, product);
}

bool bt_acio_mgr_port_packet_submit(
    bt_acio_mgr_port_dispatcher_t *dispatcher,
    bt_acio_message_t *msg,
    uint32_t max_resp_size)
{
    log_assert(_bt_acio_mgr_api_is_valid());

    return _bt_acio_mgr_api.v1.port_packet_submit(
        dispatcher, msg, max_resp_size);
}

bt_acio_drv_device_ctx_t *
bt_acio_mgr_port_checkout(bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    log_assert(_bt_acio_mgr_api_is_valid());

    return _bt_acio_mgr_api.v1.port_checkout(dispatcher);
}

void bt_acio_mgr_port_checkin(bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    log_assert(_bt_acio_mgr_api_is_valid());

    _bt_acio_mgr_api.v1.port_checkin(dispatcher);
}

#ifndef BT_API_ACIO_MGR_H
#define BT_API_ACIO_MGR_H

#include <stdbool.h>
#include <stdint.h>

#define BT_ACIO_MGR_NODE_PRODUCT_CODE_LEN 4

typedef struct ac_io_message bt_acio_message_t;
typedef struct aciodrv_device_ctx bt_acio_drv_device_ctx_t;
typedef struct bt_acio_mgr_port_dispatcher bt_acio_mgr_port_dispatcher_t;

typedef bt_acio_mgr_port_dispatcher_t *(*bt_acio_mgr_port_init_t)(const char *path, uint32_t baud);
typedef void (*bt_acio_mgr_port_fini_t)(bt_acio_mgr_port_dispatcher_t *dispatcher);
typedef uint8_t (*bt_acio_mgr_node_count_get_t)(const bt_acio_mgr_port_dispatcher_t *dispatcher);
typedef bool (*bt_acio_mgr_node_product_ident_get_t)(
    const bt_acio_mgr_port_dispatcher_t *dispatcher,
    uint8_t node_id,
    char product[BT_ACIO_MGR_NODE_PRODUCT_CODE_LEN]);
typedef bool (*bt_acio_mgr_port_packet_submit_t)(
    bt_acio_mgr_port_dispatcher_t *dispatcher,
    bt_acio_message_t *msg,
    uint32_t max_resp_size);
typedef bt_acio_drv_device_ctx_t *(*bt_acio_mgr_port_checkout_t)(bt_acio_mgr_port_dispatcher_t *dispatcher);
typedef void (*bt_acio_mgr_port_checkin_t)(bt_acio_mgr_port_dispatcher_t *dispatcher);

typedef struct bt_acio_mgr_api {
    uint16_t version;

    struct {
        // Required to be implemented
        bt_acio_mgr_port_init_t port_init;
        bt_acio_mgr_port_fini_t port_fini;
        bt_acio_mgr_node_count_get_t node_count_get;
        bt_acio_mgr_node_product_ident_get_t node_product_ident_get;
        bt_acio_mgr_port_packet_submit_t port_packet_submit;
        bt_acio_mgr_port_checkout_t port_checkout;
        bt_acio_mgr_port_checkin_t port_checkin;
    } v1;
} bt_acio_mgr_api_t;

#endif

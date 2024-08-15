#define LOG_MODULE "aciomgr-manager"

#include <windows.h>

#include <stdatomic.h>

#include "acio/acio.h"
#include "aciodrv/device.h"

#include "acio-mgr/internal.h"

#include "api/core/log.h"

#include "iface-core/log.h"

#include "sdk/module/acio/mgr.h"
#include "sdk/module/core/log.h"

#include "util/array.h"

#define MAX_PORT_PATH_LENGTH 256

typedef struct bt_acio_mgr_port_dispatcher {
    CRITICAL_SECTION cs;
    atomic_size_t references;
    struct aciodrv_device_ctx *device;
    char path[MAX_PORT_PATH_LENGTH];
    int baud;
    bool has_failure;
} bt_acio_mgr_port_dispatcher_t;

static void _bt_acio_mgr_setup_port_dispatcher(
    bt_acio_mgr_port_dispatcher_t *dispatcher, const char *path, int baud);
static void
_bt_acio_mgr_destroy_port_dispatcher(bt_acio_mgr_port_dispatcher_t *dispatcher);

// DLL-globals
static atomic_bool running;
static CRITICAL_SECTION mgr_cs;
static struct array active_ports;

static void _bt_acio_mgr_setup_port_dispatcher(
    bt_acio_mgr_port_dispatcher_t *dispatcher, const char *path, int baud)
{
    InitializeCriticalSection(&dispatcher->cs);

    strcpy(dispatcher->path, path);
    dispatcher->baud = baud;

    dispatcher->device = aciodrv_device_open_path(path, baud);
    dispatcher->has_failure = false;

    if (dispatcher->device == NULL) {
        log_info("Opening ACIO device on %s failed", path);
        dispatcher->has_failure = true;
    }

    dispatcher->references = 0;
}

static void
_bt_acio_mgr_destroy_port_dispatcher(bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    aciodrv_device_close(dispatcher->device);
    DeleteCriticalSection(&dispatcher->cs);
}

void bt_acio_mgr_init()
{
    if (running) {
        log_warning("_aciomgr_init called when already running");
        bt_acio_mgr_fini();
    }
    InitializeCriticalSection(&mgr_cs);
    array_init(&active_ports);

    running = true;
}

void bt_acio_mgr_fini()
{
    if (!running) {
        log_warning("_aciomgr_fini called when not running");
    }

    running = false;
    DeleteCriticalSection(&mgr_cs);
    array_fini(&active_ports);
}

bt_acio_mgr_port_dispatcher_t *
bt_acio_mgr_port_init(const char *path, uint32_t baud)
{
    if (!running) {
        log_warning("aciomgr_port_init: called when not running");
        return NULL;
    }

    if (strlen(path) >= MAX_PORT_PATH_LENGTH) {
        log_warning("aciomgr_port_init: path too long");
        return NULL;
    }

    bt_acio_mgr_port_dispatcher_t *entry;

    EnterCriticalSection(&mgr_cs);

    for (size_t i = 0; i < active_ports.nitems; i++) {
        entry = array_item(bt_acio_mgr_port_dispatcher_t, &active_ports, i);

        if (strcmp(entry->path, path) == 0) {
            // found
            if (entry->baud != baud) {
                log_warning(
                    "Device at %s opened with baud of %d, but requested %d",
                    path,
                    entry->baud,
                    baud);
            }
            goto done;
        }
    }

    entry = array_append(bt_acio_mgr_port_dispatcher_t, &active_ports);
    _bt_acio_mgr_setup_port_dispatcher(entry, path, baud);

done:
    if (!entry->has_failure) {
        entry->references++;
    }

    LeaveCriticalSection(&mgr_cs);

    if (entry->has_failure) {
        return NULL;
    }

    return entry;
}

void bt_acio_mgr_port_fini(bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    if (!running) {
        log_warning("aciomgr_port_fini: called when not running");
        return;
    }

    EnterCriticalSection(&mgr_cs);
    dispatcher->references--;
    LeaveCriticalSection(&mgr_cs);

    if (dispatcher->references == 0) {
        _bt_acio_mgr_destroy_port_dispatcher(dispatcher);
    }
}

// this function don't require the lock
uint8_t
bt_acio_mgr_node_count_get(const bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    return aciodrv_device_get_node_count(dispatcher->device);
}

// this function don't require the lock
bool bt_acio_mgr_node_product_ident_get(
    const bt_acio_mgr_port_dispatcher_t *dispatcher,
    uint8_t node_id,
    char product[BT_ACIO_MGR_NODE_PRODUCT_CODE_LEN])
{
    return aciodrv_device_get_node_product_ident(
        dispatcher->device, node_id, product);
}

bool bt_acio_mgr_port_packet_submit(
    bt_acio_mgr_port_dispatcher_t *dispatcher,
    bt_acio_message_t *msg,
    uint32_t max_resp_size)
{
    // CS's although lightweight, may still be a burden, short circuit
    if (dispatcher->references > 1) {
        EnterCriticalSection(&dispatcher->cs);
        bool response =
            aciodrv_send_and_recv(dispatcher->device, msg, max_resp_size);
        LeaveCriticalSection(&dispatcher->cs);
        return response;
    }

    return aciodrv_send_and_recv(dispatcher->device, msg, max_resp_size);
}

bt_acio_drv_device_ctx_t *
bt_acio_mgr_port_checkout(bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    if (dispatcher->references > 1) {
        EnterCriticalSection(&dispatcher->cs);
    }

    return dispatcher->device;
}

void bt_acio_mgr_port_checkin(bt_acio_mgr_port_dispatcher_t *dispatcher)
{
    if (dispatcher->references > 1) {
        LeaveCriticalSection(&dispatcher->cs);
    }
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_acio_mgr_api_get(bt_acio_mgr_api_t *api)
{
    api->version = 1;

    api->v1.port_init = bt_acio_mgr_port_init;
    api->v1.port_fini = bt_acio_mgr_port_fini;
    api->v1.node_count_get = bt_acio_mgr_node_count_get;
    api->v1.node_product_ident_get = bt_acio_mgr_node_product_ident_get;
    api->v1.port_packet_submit = bt_acio_mgr_port_packet_submit;
    api->v1.port_checkout = bt_acio_mgr_port_checkout;
    api->v1.port_checkin = bt_acio_mgr_port_checkin;
}
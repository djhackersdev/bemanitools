#define LOG_MODULE "aciomgr-manager"

#include <windows.h>

#include <stdatomic.h>

#include "manager.h"

#include "aciodrv/device.h"
#include "util/array.h"
#include "util/log.h"

#define MAX_PORT_PATH_LENGTH 256

struct aciomgr_port_dispatcher {
    CRITICAL_SECTION cs;
    size_t references;
    struct aciodrv_device_ctx *device;
    char path[MAX_PORT_PATH_LENGTH];
    int baud;
};

static void _aciomgr_setup_port_dispatcher(
    struct aciomgr_port_dispatcher *dispatcher, const char *path, int baud);
static void _aciomgr_destroy_port_dispatcher(
    struct aciomgr_port_dispatcher *dispatcher);
static void _aciomgr_init();
static void _aciomgr_fini();

// DLL-globals
static atomic_bool running;
static CRITICAL_SECTION mgr_cs;
static struct array active_ports;

static void _aciomgr_setup_port_dispatcher(
    struct aciomgr_port_dispatcher *dispatcher, const char *path, int baud)
{
    InitializeCriticalSection(&dispatcher->cs);

    strcpy(dispatcher->path, path);
    dispatcher->baud = baud;

    dispatcher->device = aciodrv_device_open_path(path, baud);
    dispatcher->references = 0;
}

static void _aciomgr_destroy_port_dispatcher(
    struct aciomgr_port_dispatcher *dispatcher)
{
    aciodrv_device_close(dispatcher->device);
    DeleteCriticalSection(&dispatcher->cs);
}

static void _aciomgr_init()
{
    if (running) {
        // warn
        _aciomgr_fini();
    }
    InitializeCriticalSection(&mgr_cs);
    array_init(&active_ports);

    running = true;
}

static void _aciomgr_fini()
{
    if (!running) {
        // warn
    }
    running = false;
    DeleteCriticalSection(&mgr_cs);
    array_fini(&active_ports);
}

void aciomgr_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    log_to_external(misc, info, warning, fatal);
}

struct aciomgr_port_dispatcher *aciomgr_port_init(const char *path, int baud)
{
    if (!running) {
        // warn
        return NULL;
    }

    if (strlen(path) >= MAX_PORT_PATH_LENGTH) {
        // warn
        return NULL;
    }


    struct aciomgr_port_dispatcher* entry;

    EnterCriticalSection(&mgr_cs);

    for (size_t i = 0; i < active_ports.nitems; i++) {
        entry = array_item(struct aciomgr_port_dispatcher, &active_ports, i);

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

    entry = array_append(struct aciomgr_port_dispatcher, &active_ports);
    _aciomgr_setup_port_dispatcher(entry, path, baud);

done:
    entry->references++;
    LeaveCriticalSection(&mgr_cs);

    return entry;
}

void aciomgr_port_fini(struct aciomgr_port_dispatcher *dispatcher)
{
    if (!running) {
        // warn
        return;
    }

    EnterCriticalSection(&mgr_cs);
    dispatcher->references--;
    LeaveCriticalSection(&mgr_cs);

    if (dispatcher->references == 0) {
        _aciomgr_destroy_port_dispatcher(dispatcher);
    }
}

// this function don't require the lock
uint8_t aciomgr_get_node_count(struct aciomgr_port_dispatcher *dispatcher) {
    return aciodrv_device_get_node_count(dispatcher->device);
}

// this function don't require the lock
bool aciomgr_get_node_product_ident(
    struct aciomgr_port_dispatcher *dispatcher,
    uint8_t node_id,
    char product[4])
{
    return aciodrv_device_get_node_product_ident(dispatcher->device, node_id, product);
}

bool aciomgr_port_submit_packet(
    struct aciomgr_port_dispatcher *dispatcher,
    struct ac_io_message *msg,
    int max_resp_size)
{
    // CS's although lightweight, may still be a burden, short circuit
    if (dispatcher->references > 1) {
        EnterCriticalSection(&dispatcher->cs);
        bool response = aciodrv_send_and_recv(dispatcher->device, msg, max_resp_size);
        LeaveCriticalSection(&dispatcher->cs);
        return response;
    }

    return aciodrv_send_and_recv(dispatcher->device, msg, max_resp_size);
}

struct aciodrv_device_ctx *aciomgr_port_checkout(
    struct aciomgr_port_dispatcher *dispatcher)
{
    if (dispatcher->references > 1) {
        EnterCriticalSection(&dispatcher->cs);
    }

    return dispatcher->device;
}

void aciomgr_port_checkin(struct aciomgr_port_dispatcher *dispatcher) {
    if (dispatcher->references > 1) {
        LeaveCriticalSection(&dispatcher->cs);
    }
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {
        _aciomgr_init();
    }

    if (reason == DLL_PROCESS_DETACH) {
        _aciomgr_fini();
    }

    return TRUE;
}

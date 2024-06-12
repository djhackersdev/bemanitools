#define LOG_MODULE "ddrio-async"

#include <windows.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "iface-core/log.h"
#include "iface-core/thread.h"
#include "iface-io/ddr.h"

#include "module/io-ext.h"
#include "module/io.h"

#include "util/time.h"

#include "sdk/module/core/log.h"
#include "sdk/module/core/thread.h"
#include "sdk/module/io/ddr.h"

static module_io_t *_ddr_io_async_module_ddr_io;

static bt_core_thread_id_t _io_thread_id;
static _Atomic(bool) _io_thread_proc_loop;
static _Atomic(bool) _io_thread_proc_running;

static _Atomic(uint32_t) _child_ddr_io_data_pad;
static _Atomic(uint32_t) _child_ddr_io_data_extio_lights;
static _Atomic(uint32_t) _child_ddr_io_data_p3io_lights;

static int _io_thread_proc(void *ctx)
{
    uint64_t time_start;
    uint64_t time_end;
    uint64_t loop_counter;
    uint64_t total_time;

    uint32_t prev_child_ddr_io_data_extio_lights;
    uint32_t prev_child_ddr_io_data_p3io_lights;

    uint32_t local_tmp;

    atomic_store_explicit(&_io_thread_proc_running, true, memory_order_seq_cst);

    log_info("IO thread running");

    prev_child_ddr_io_data_extio_lights = atomic_load_explicit(
        &_child_ddr_io_data_extio_lights, memory_order_seq_cst);
    prev_child_ddr_io_data_p3io_lights = atomic_load_explicit(
        &_child_ddr_io_data_p3io_lights, memory_order_seq_cst);

    time_start = time_get_counter();
    loop_counter = 0;

    while (atomic_load_explicit(&_io_thread_proc_loop, memory_order_seq_cst)) {
        local_tmp = bt_io_ddr_pad_read();

        atomic_store_explicit(
            &_child_ddr_io_data_pad, local_tmp, memory_order_relaxed);

        // Only update outputs when they change gives this loop a major
        // performance boost The write calls on a p3io for the outputs are very
        // expensive (~12 ms) as they are executed over the ACIO protocol
        // compared to only the input read calls (~4 ms) which have a dedicated
        // IOCTL call/endpoint

        local_tmp = atomic_load_explicit(
            &_child_ddr_io_data_extio_lights, memory_order_relaxed);

        if (local_tmp != prev_child_ddr_io_data_extio_lights) {
            bt_io_ddr_extio_lights_set(local_tmp);
            prev_child_ddr_io_data_extio_lights = local_tmp;
        }

        local_tmp = atomic_load_explicit(
            &_child_ddr_io_data_p3io_lights, memory_order_relaxed);

        if (local_tmp != prev_child_ddr_io_data_p3io_lights) {
            bt_io_ddr_p3io_lights_set(local_tmp);
            prev_child_ddr_io_data_p3io_lights = local_tmp;
        }

        // Don't hog the CPU
        SwitchToThread();

        loop_counter++;
    }

    time_end = time_get_counter();
    total_time = time_get_elapsed_us(time_end - time_start);

    log_info(
        "IO thread performance: total iterations %lld, avg. loop cycle time %f "
        "us",
        loop_counter,
        ((double) total_time) / loop_counter);

    atomic_store_explicit(
        &_io_thread_proc_running, false, memory_order_seq_cst);

    log_info("IO thread shut down");

    return 0;
}

static void _ddr_io_async_child_ddr_io_init(module_io_t **module)
{
    bt_io_ddr_api_t api;

    module_io_ext_load_and_init(
        "ddrio-async-child.dll", "bt_module_io_ddr_api_get", module);
    module_io_api_get(*module, &api);
    bt_io_ddr_api_set(&api);
}

bool bt_io_ddr_async_init()
{
    bool result;
    bt_core_thread_result_t result2;

    log_info("Loading ddrio-async-child.dll as child ddrio library...");

    _ddr_io_async_child_ddr_io_init(&_ddr_io_async_module_ddr_io);

    log_info("Calling child ddr_io_init...");

    result = bt_io_ddr_init();

    if (!result) {
        log_warning("Child ddr_io_init failed");
        module_io_free(&_ddr_io_async_module_ddr_io);

        return false;
    }

    atomic_store_explicit(&_io_thread_proc_loop, true, memory_order_seq_cst);

    result2 =
        bt_core_thread_create(_io_thread_proc, NULL, 16384, 0, &_io_thread_id);

    if (result2 != BT_CORE_THREAD_RESULT_SUCCESS) {
        log_warning(
            "Creating IO thread failed: %s",
            bt_core_thread_result_to_str(result2));

        bt_io_ddr_fini();
        module_io_free(&_ddr_io_async_module_ddr_io);

        return false;
    }

    return true;
}

void bt_io_ddr_async_fini(void)
{
    atomic_store_explicit(&_io_thread_proc_loop, false, memory_order_seq_cst);

    log_info("Shutting down IO thread and waiting for it to finish...");

    while (
        atomic_load_explicit(&_io_thread_proc_running, memory_order_seq_cst)) {
        Sleep(1);
    }

    log_info("IO thread finished");

    bt_io_ddr_fini();

    module_io_free(&_ddr_io_async_module_ddr_io);
}

uint32_t bt_io_ddr_async_pad_read(void)
{
    return atomic_load_explicit(&_child_ddr_io_data_pad, memory_order_relaxed);
}

void bt_io_ddr_async_extio_lights_set(uint32_t extio_lights)
{
    atomic_store_explicit(
        &_child_ddr_io_data_extio_lights, extio_lights, memory_order_relaxed);
}

void bt_io_ddr_async_p3io_lights_set(uint32_t p3io_lights)
{
    atomic_store_explicit(
        &_child_ddr_io_data_p3io_lights, p3io_lights, memory_order_relaxed);
}

void bt_io_ddr_async_hdxs_lights_panel_set(uint32_t lights)
{
    // Not implemented for now
}

void bt_io_ddr_async_hdxs_lights_rgb_set(
    uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    // Not implemented for now
}

void bt_module_core_log_api_set(const bt_core_log_api_t *api)
{
    bt_core_log_api_set(api);
}

void bt_module_core_thread_api_set(const bt_core_thread_api_t *api)
{
    bt_core_thread_api_set(api);
}

void bt_module_io_ddr_api_get(bt_io_ddr_api_t *api)
{
    api->version = 1;

    api->v1.init = bt_io_ddr_async_init;
    api->v1.fini = bt_io_ddr_async_fini;
    api->v1.pad_read = bt_io_ddr_async_pad_read;
    api->v1.extio_lights_set = bt_io_ddr_async_extio_lights_set;
    api->v1.p3io_lights_set = bt_io_ddr_async_p3io_lights_set;
    api->v1.hdxs_lights_panel_set = bt_io_ddr_async_hdxs_lights_panel_set;
    api->v1.hdxs_lights_rgb_set = bt_io_ddr_async_hdxs_lights_rgb_set;
}
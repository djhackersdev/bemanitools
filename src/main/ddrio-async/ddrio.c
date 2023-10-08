#define LOG_MODULE "ddrio-async"

#include <windows.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "bemanitools/ddrio.h"

#include "util/log.h"
#include "util/thread.h"
#include "util/time.h"

typedef void (*ddr_io_set_loggers_t)(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);
typedef bool (*ddr_io_init_t)(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);
typedef uint32_t (*ddr_io_read_pad_t)(void);
typedef void (*ddr_io_set_lights_extio_t)(uint32_t extio_lights);
typedef void (*ddr_io_set_lights_p3io_t)(uint32_t p3io_lights);
typedef void (*ddr_io_set_lights_hdxs_panel_t)(uint32_t hdxs_lights);
typedef void (*ddr_io_set_lights_hdxs_rgb_t)(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);
typedef void (*ddr_io_fini_t)(void);

static HMODULE _child_ddr_io_module;

static ddr_io_set_loggers_t _child_ddr_io_set_loggers;
static ddr_io_init_t _child_ddr_io_init;
static ddr_io_read_pad_t _child_ddr_io_read_pad;
static ddr_io_set_lights_extio_t _child_ddr_io_set_lights_extio;
static ddr_io_set_lights_p3io_t _child_ddr_io_set_lights_p3io;
static ddr_io_set_lights_hdxs_panel_t _child_ddr_io_set_lights_hdxs_panel;
static ddr_io_set_lights_hdxs_rgb_t _child_ddr_io_set_lights_hdxs_rgb;
static ddr_io_fini_t _child_ddr_io_fini;

static log_formatter_t _log_formatter_misc;
static log_formatter_t _log_formatter_info;
static log_formatter_t _log_formatter_warning;
static log_formatter_t _log_formatter_fatal;

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

    atomic_store_explicit(
            &_io_thread_proc_running,
            true,
            memory_order_seq_cst);

    log_info("IO thread running");

    prev_child_ddr_io_data_extio_lights = atomic_load_explicit(
        &_child_ddr_io_data_extio_lights, memory_order_seq_cst);
    prev_child_ddr_io_data_p3io_lights = atomic_load_explicit(
        &_child_ddr_io_data_p3io_lights, memory_order_seq_cst);

    time_start = time_get_counter();
    loop_counter = 0;

    while (atomic_load_explicit(&_io_thread_proc_loop, memory_order_seq_cst)) {
        local_tmp = _child_ddr_io_read_pad();

        atomic_store_explicit(
            &_child_ddr_io_data_pad,
            local_tmp,
            memory_order_relaxed);

        // Only update outputs when they change gives this loop a major performance boost
        // The write calls on a p3io for the outputs are very expensive (~12 ms) as they
        // are executed over the ACIO protocol compared to only the input read
        // calls (~4 ms) which have a dedicated IOCTL call/endpoint

        local_tmp = atomic_load_explicit(
            &_child_ddr_io_data_extio_lights,
            memory_order_relaxed);

        if (local_tmp != prev_child_ddr_io_data_extio_lights) {
            _child_ddr_io_set_lights_extio(local_tmp);
            prev_child_ddr_io_data_extio_lights = local_tmp;
        }

        local_tmp = atomic_load_explicit(
            &_child_ddr_io_data_p3io_lights,
            memory_order_relaxed);
        
        if (local_tmp != prev_child_ddr_io_data_p3io_lights) {
            _child_ddr_io_set_lights_p3io(local_tmp);
            prev_child_ddr_io_data_p3io_lights = local_tmp;
        }

        // Don't hog the CPU
        SwitchToThread();

        loop_counter++;
    }

    time_end = time_get_counter();
    total_time = time_get_elapsed_us(time_end - time_start);

    log_info("IO thread performance: total iterations %lld, avg. loop cycle time %f us",
        loop_counter, ((double) total_time) / loop_counter);

    atomic_store_explicit(
            &_io_thread_proc_running,
            false,
            memory_order_seq_cst);

    log_info("IO thread shut down");

    return 0;
}

static void* _load_function(HMODULE module, const char* name)
{
    void* ptr;

    ptr = GetProcAddress(module, name);

    if (ptr == NULL) {
        log_fatal("Could not find function %s in ddr3io child library", name);
    }

    return ptr;
}

void ddr_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    _log_formatter_misc = misc;
    _log_formatter_info = info;
    _log_formatter_warning = warning;
    _log_formatter_fatal = fatal;

    log_to_external(misc, info, warning, fatal);
}

bool ddr_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    log_info("Loading ddrio-async-child.dll as child ddrio library...");

    _child_ddr_io_module = LoadLibraryA("ddrio-async-child.dll");

    if (_child_ddr_io_module == NULL) {
        log_warning("Loading ddrio-async-child.dll failed");
        return false;
    }

    _child_ddr_io_set_loggers = _load_function(_child_ddr_io_module, "ddr_io_set_loggers");
    _child_ddr_io_init = _load_function(_child_ddr_io_module, "ddr_io_init");
    _child_ddr_io_read_pad = _load_function(_child_ddr_io_module, "ddr_io_read_pad");
    _child_ddr_io_set_lights_extio = _load_function(_child_ddr_io_module, "ddr_io_set_lights_extio");
    _child_ddr_io_set_lights_p3io = _load_function(_child_ddr_io_module, "ddr_io_set_lights_p3io");
    _child_ddr_io_set_lights_hdxs_panel = _load_function(_child_ddr_io_module, "ddr_io_set_lights_hdxs_panel");
    _child_ddr_io_set_lights_hdxs_rgb = _load_function(_child_ddr_io_module, "ddr_io_set_lights_hdxs_rgb");
    _child_ddr_io_fini = _load_function(_child_ddr_io_module, "ddr_io_fini");

    _child_ddr_io_set_loggers(
        _log_formatter_misc,
        _log_formatter_info,
        _log_formatter_warning,
        _log_formatter_fatal);

    log_info("Calling child ddr_io_init...");

    if (!_child_ddr_io_init(thread_create, thread_join, thread_destroy)) {
        log_warning("Child ddr_io_init failed");
        FreeLibrary(_child_ddr_io_module);

        return false;
    }

    atomic_store_explicit(
            &_io_thread_proc_loop,
            true,
            memory_order_seq_cst);

    if (!thread_create(_io_thread_proc, NULL, 16384, 0)) {
        log_warning("Creating IO thread failed");

        _child_ddr_io_fini();
        FreeLibrary(_child_ddr_io_module);

        return false;
    }

    return true;
}

uint32_t ddr_io_read_pad(void)
{
    return atomic_load_explicit(&_child_ddr_io_data_pad, memory_order_relaxed);
}

void ddr_io_set_lights_extio(uint32_t extio_lights)
{
    atomic_store_explicit(
            &_child_ddr_io_data_extio_lights,
            extio_lights,
            memory_order_relaxed);
}

void ddr_io_set_lights_p3io(uint32_t p3io_lights)
{
    atomic_store_explicit(
            &_child_ddr_io_data_p3io_lights,
            p3io_lights,
            memory_order_relaxed);
}

void ddr_io_set_lights_hdxs_panel(uint32_t lights)
{
    // Not implemented for now
}

void ddr_io_set_lights_hdxs_rgb(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    // Not implemented for now
}

void ddr_io_fini(void)
{
    atomic_store_explicit(
            &_io_thread_proc_loop,
            false,
            memory_order_seq_cst);

    log_info("Shutting down IO thread and waiting for it to finish...");

    while (atomic_load_explicit(&_io_thread_proc_running, memory_order_seq_cst)) {
        Sleep(1);
    }

    log_info("IO thread finished");

    _child_ddr_io_fini();

    FreeLibrary(_child_ddr_io_module);
}

#define LOG_MODULE "iidxio-async"

#include <windows.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "bemanitools/iidxio.h"

#include "util/log.h"
#include "util/thread.h"
#include "util/time.h"

typedef void (*iidx_io_set_loggers_t)(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);
typedef bool (*iidx_io_init_t)(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);
typedef void (*iidx_io_fini_t)(void);
typedef void (*iidx_io_ep1_set_deck_lights_t)(uint16_t deck_lights);
typedef void (*iidx_io_ep1_set_panel_lights_t)(uint8_t panel_lights);
typedef void (*iidx_io_ep1_set_top_lamps_t)(uint8_t top_lamps);
typedef void (*iidx_io_ep1_set_top_neons_t)(bool top_neons);
typedef bool (*iidx_io_ep1_send_t)(void);
typedef bool (*iidx_io_ep2_recv_t)(void);
typedef uint8_t (*iidx_io_ep2_get_turntable_t)(uint8_t player_no);
typedef uint8_t (*iidx_io_ep2_get_slider_t)(uint8_t slider_no);
typedef uint8_t (*iidx_io_ep2_get_sys_t)(void);
typedef uint8_t (*iidx_io_ep2_get_panel_t)(void);
typedef uint16_t (*iidx_io_ep2_get_keys_t)(void);
typedef bool (*iidx_io_ep3_write_16seg_t)(const char *text);

static HMODULE _child_iidx_io_module;

static iidx_io_set_loggers_t _child_iidx_io_set_loggers;
static iidx_io_init_t _child_iidx_io_init;
static iidx_io_fini_t _child_iidx_io_fini;

static iidx_io_ep1_set_deck_lights_t _child_iidx_io_ep1_set_deck_lights;
static iidx_io_ep1_set_panel_lights_t _child_iidx_io_ep1_set_panel_lights;
static iidx_io_ep1_set_top_lamps_t _child_iidx_io_ep1_set_top_lamps;
static iidx_io_ep1_set_top_neons_t _child_iidx_io_ep1_set_top_neons;
static iidx_io_ep1_send_t _child_iidx_io_ep1_send;
static iidx_io_ep2_recv_t _child_iidx_io_ep2_recv;
static iidx_io_ep2_get_turntable_t _child_iidx_io_ep2_get_turntable;
static iidx_io_ep2_get_slider_t _child_iidx_io_ep2_get_slider;
static iidx_io_ep2_get_sys_t _child_iidx_io_ep2_get_sys;
static iidx_io_ep2_get_panel_t _child_iidx_io_ep2_get_panel;
static iidx_io_ep2_get_keys_t _child_iidx_io_ep2_get_keys;
static iidx_io_ep3_write_16seg_t _child_iidx_io_ep3_write_16seg;

static log_formatter_t _log_formatter_misc;
static log_formatter_t _log_formatter_info;
static log_formatter_t _log_formatter_warning;
static log_formatter_t _log_formatter_fatal;

static _Atomic(bool) _io_thread_proc_loop;
static _Atomic(bool) _io_thread_proc_running;

static _Atomic(uint16_t) _child_iidx_io_deck_lights;
static _Atomic(uint8_t) _child_iidx_io_panel_lights;
static _Atomic(uint8_t) _child_iidx_io_top_lamps;
static _Atomic(bool) _child_iidx_io_top_neons;

static _Atomic(uint8_t) _child_iidx_io_turntable_p1;
static _Atomic(uint8_t) _child_iidx_io_turntable_p2;
static _Atomic(uint8_t) _child_iidx_io_slider_1;
static _Atomic(uint8_t) _child_iidx_io_slider_2;
static _Atomic(uint8_t) _child_iidx_io_slider_3;
static _Atomic(uint8_t) _child_iidx_io_slider_4;
static _Atomic(uint8_t) _child_iidx_io_slider_5;
static _Atomic(uint8_t) _child_iidx_io_sys;
static _Atomic(uint8_t) _child_iidx_io_panel;
static _Atomic(uint16_t) _child_iidx_io_keys;

static int _io_thread_proc(void *ctx)
{
    uint64_t time_start;
    uint64_t time_end;
    uint64_t loop_counter;
    uint64_t total_time;

    bool result;

    // TODO compare previous outputs with new ones and only apply if changed? doesn't make sense for iidx as setting the values is not supposed to trigger any actual IO driving
    // uint32_t prev_child_ddr_io_data_extio_lights;
    // uint32_t prev_child_ddr_io_data_p3io_lights;

    // uint32_t local_tmp;

    atomic_store_explicit(&_io_thread_proc_running, true, memory_order_seq_cst);

    log_info("IO thread running");

    time_start = time_get_counter();
    loop_counter = 0;

    // TODO have a setting to configure polling rates of inputs, outputs and 16seg separately
    while (atomic_load_explicit(&_io_thread_proc_loop, memory_order_seq_cst)) {
        // TODO measure time of expensive calls independently
        result = _child_iidx_io_ep2_recv();

        if (!result) {
            log_warning("_child_iidx_io_ep2_recv returned false");
            atomic_store_explicit(
                &_io_thread_proc_running, false, memory_order_seq_cst);

            log_info("IO thread shut down");

            return 0;
        }

        atomic_store_explicit(&_child_iidx_io_turntable_p1, _child_iidx_io_ep2_get_turntable(0), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_turntable_p2, _child_iidx_io_ep2_get_turntable(1), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_slider_1, _child_iidx_io_ep2_get_slider(0), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_slider_2, _child_iidx_io_ep2_get_slider(1), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_slider_3, _child_iidx_io_ep2_get_slider(2), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_slider_4, _child_iidx_io_ep2_get_slider(3), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_slider_5, _child_iidx_io_ep2_get_slider(4), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_sys, _child_iidx_io_ep2_get_sys(), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_panel, _child_iidx_io_ep2_get_panel(), memory_order_relaxed);
        atomic_store_explicit(&_child_iidx_io_keys, _child_iidx_io_ep2_get_keys(), memory_order_relaxed);

        _child_iidx_io_ep1_set_deck_lights(atomic_load_explicit(&_child_iidx_io_deck_lights, memory_order_relaxed));
        _child_iidx_io_ep1_set_panel_lights(atomic_load_explicit(&_child_iidx_io_panel_lights, memory_order_relaxed));
        _child_iidx_io_ep1_set_top_lamps(atomic_load_explicit(&_child_iidx_io_top_lamps, memory_order_relaxed));
        _child_iidx_io_ep1_set_top_neons(atomic_load_explicit(&_child_iidx_io_top_neons, memory_order_relaxed));

        // TODO guard and only update every X loops? -> configurable?
        result = _child_iidx_io_ep1_send();

        if (!result) {
            log_warning("_child_iidx_io_ep1_send returned false");
            atomic_store_explicit(
                &_io_thread_proc_running, false, memory_order_seq_cst);

            log_info("IO thread shut down");

            return 0;
        }

        // TODO atomic write of 16seg with mutex?
        // result = _child_iidx_io_ep3_write_16seg();

        // if (!result) {
        //     log_warning("_child_iidx_io_ep3_write_16seg returned false");
        //     atomic_store_explicit(
        //         &_io_thread_proc_running, false, memory_order_seq_cst);

        //     log_info("IO thread shut down");

        //     return 0;
        // }

        // local_tmp = _child_ddr_io_read_pad();

        // atomic_store_explicit(
        //     &_child_ddr_io_data_pad, local_tmp, memory_order_relaxed);


        // local_tmp = atomic_load_explicit(
        //     &_child_ddr_io_data_extio_lights, memory_order_relaxed);

        // if (local_tmp != prev_child_ddr_io_data_extio_lights) {
        //     _child_ddr_io_set_lights_extio(local_tmp);
        //     prev_child_ddr_io_data_extio_lights = local_tmp;
        // }

        // local_tmp = atomic_load_explicit(
        //     &_child_ddr_io_data_p3io_lights, memory_order_relaxed);

        // if (local_tmp != prev_child_ddr_io_data_p3io_lights) {
        //     _child_ddr_io_set_lights_p3io(local_tmp);
        //     prev_child_ddr_io_data_p3io_lights = local_tmp;
        // }

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

static void *_load_function(HMODULE module, const char *name)
{
    void *ptr;

    ptr = GetProcAddress(module, name);

    if (ptr == NULL) {
        log_fatal("Could not find function %s in iidxio child library", name);
    }

    return ptr;
}

void iidx_io_set_loggers(
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

bool iidx_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    log_info("Loading iidxio-async-child.dll as child iidxio library...");

    _child_iidx_io_module = LoadLibraryA("iidxio-async-child.dll");

    if (_child_iidx_io_module == NULL) {
        log_warning("Loading iidxio-async-child.dll failed");
        return false;
    }

    _child_iidx_io_set_loggers =
        _load_function(_child_iidx_io_module, "iidx_io_set_loggers");
    _child_iidx_io_init = _load_function(_child_iidx_io_module, "iidx_io_init");
    _child_iidx_io_fini = _load_function(_child_iidx_io_module, "iidx_io_fini");

    _child_iidx_io_ep1_set_deck_lights =
        _load_function(_child_iidx_io_module, "iidx_io_ep1_set_deck_lights");
    _child_iidx_io_ep1_set_panel_lights =
        _load_function(_child_iidx_io_module, "iidx_io_ep1_set_panel_lights");
    _child_iidx_io_ep1_set_top_lamps =
        _load_function(_child_iidx_io_module, "iidx_io_ep1_set_top_lamps");
    _child_iidx_io_ep1_set_top_neons =
        _load_function(_child_iidx_io_module, "iidx_io_ep1_set_top_neons");
    _child_iidx_io_ep1_send =
        _load_function(_child_iidx_io_module, "iidx_io_ep1_send");
    _child_iidx_io_ep2_recv =
        _load_function(_child_iidx_io_module, "iidx_io_ep2_recv");
    _child_iidx_io_ep2_get_turntable =
        _load_function(_child_iidx_io_module, "iidx_io_ep2_get_turntable");
    _child_iidx_io_ep2_get_slider =
        _load_function(_child_iidx_io_module, "iidx_io_ep2_get_slider");
    _child_iidx_io_ep2_get_sys =
        _load_function(_child_iidx_io_module, "iidx_io_ep2_get_sys");
    _child_iidx_io_ep2_get_panel =
        _load_function(_child_iidx_io_module, "iidx_io_ep2_get_panel");
    _child_iidx_io_ep2_get_keys =
        _load_function(_child_iidx_io_module, "iidx_io_ep2_get_keys");
    _child_iidx_io_ep3_write_16seg =
        _load_function(_child_iidx_io_module, "iidx_io_ep3_write_16seg");

    _child_iidx_io_set_loggers(
        _log_formatter_misc,
        _log_formatter_info,
        _log_formatter_warning,
        _log_formatter_fatal);

    log_info("Calling child iidx_io_init...");

    if (!_child_iidx_io_init(thread_create, thread_join, thread_destroy)) {
        log_warning("Child iidx_io_init failed");
        FreeLibrary(_child_iidx_io_module);

        return false;
    }

    atomic_store_explicit(&_io_thread_proc_loop, true, memory_order_seq_cst);

    if (!thread_create(_io_thread_proc, NULL, 16384, 0)) {
        log_warning("Creating IO thread failed");

        _child_iidx_io_fini();
        FreeLibrary(_child_iidx_io_module);

        return false;
    }

    return true;
}

void iidx_io_fini(void)
{
    atomic_store_explicit(&_io_thread_proc_loop, false, memory_order_seq_cst);

    log_info("Shutting down IO thread and waiting for it to finish...");

    while (
        atomic_load_explicit(&_io_thread_proc_running, memory_order_seq_cst)) {
        Sleep(1);
    }

    log_info("IO thread finished");

    _child_iidx_io_fini();

    FreeLibrary(_child_iidx_io_module);
}

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights)
{
    atomic_store_explicit(
        &_child_iidx_io_deck_lights, deck_lights, memory_order_relaxed);
}

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights)
{
    atomic_store_explicit(
        &_child_iidx_io_panel_lights, panel_lights, memory_order_relaxed);
}

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps)
{
    atomic_store_explicit(
        &_child_iidx_io_top_lamps, top_lamps, memory_order_relaxed);
}

void iidx_io_ep1_set_top_neons(bool top_neons)
{
    atomic_store_explicit(
        &_child_iidx_io_top_neons, top_neons, memory_order_relaxed);
}

bool iidx_io_ep1_send(void)
{
    // Any sending and receiving is executed async in a separate thread
    return true;
}

bool iidx_io_ep2_recv(void)
{
    // Any sending and receiving is executed async in a separate thread
    return true;
}

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no)
{
    switch (player_no)
    {
        case 0:
            return atomic_load_explicit(&_child_iidx_io_turntable_p1, memory_order_relaxed);
        case 1:
            return atomic_load_explicit(&_child_iidx_io_turntable_p2, memory_order_relaxed);
        default:
            return 0;
    }
}

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no)
{
    switch (slider_no)
    {
        case 0: 
            return atomic_load_explicit(&_child_iidx_io_slider_1, memory_order_relaxed);
        case 1: 
            return atomic_load_explicit(&_child_iidx_io_slider_2, memory_order_relaxed);
        case 2: 
            return atomic_load_explicit(&_child_iidx_io_slider_3, memory_order_relaxed);
        case 3:
            return atomic_load_explicit(&_child_iidx_io_slider_4, memory_order_relaxed);
        case 4:
            return atomic_load_explicit(&_child_iidx_io_slider_5, memory_order_relaxed);
        default:
            return 0;
    }
}

uint8_t iidx_io_ep2_get_sys(void)
{
    return atomic_load_explicit(&_child_iidx_io_sys, memory_order_relaxed);
}

uint8_t iidx_io_ep2_get_panel(void)
{
    return atomic_load_explicit(&_child_iidx_io_panel, memory_order_relaxed);
}

uint16_t iidx_io_ep2_get_keys(void)
{
    return atomic_load_explicit(&_child_iidx_io_keys, memory_order_relaxed);
}

bool iidx_io_ep3_write_16seg(const char *text)
{
    // TODO atomic copy with mutex?

    return true;
}
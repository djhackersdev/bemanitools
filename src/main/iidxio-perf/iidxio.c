#define LOG_MODULE "iidxio-perf"

#include <windows.h>

#include <inttypes.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "bemanitools/iidxio.h"

#include "hdr-histogram/hdr_histogram.h"
#include "hdr-histogram/hdr_time.h"

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

struct hdr_histogram* _histogram_ep1_send;
struct hdr_histogram* _histogram_ep2_recv;
struct hdr_histogram* _histogram_ep3_write_16seg;

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
    size_t histogram_memory_usage_bytes;

    log_info("Loading iidxio-perf-child.dll as child iidxio library...");

    _child_iidx_io_module = LoadLibraryA("iidxio-perf-child.dll");

    if (_child_iidx_io_module == NULL) {
        log_warning("Loading iidxio-perf-child.dll failed");
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

    log_info("Initialising histograms...");

    hdr_init(1, INT64_C(3600000000), 3, &_histogram_ep1_send);
    hdr_init(1, INT64_C(3600000000), 3, &_histogram_ep2_recv);
    hdr_init(1, INT64_C(3600000000), 3, &_histogram_ep3_write_16seg);

    histogram_memory_usage_bytes =
        hdr_get_memory_size(_histogram_ep1_send) +
        hdr_get_memory_size(_histogram_ep2_recv) +
        hdr_get_memory_size(_histogram_ep3_write_16seg);

    log_misc("Histogram total memory usage: %" PRIdPTR " bytes", histogram_memory_usage_bytes);

    log_info("Calling child iidx_io_init...");

    if (!_child_iidx_io_init(thread_create, thread_join, thread_destroy)) {
        log_warning("Child iidx_io_init failed");
        FreeLibrary(_child_iidx_io_module);

        return false;
    } else {
        return true;
    }
}

void iidx_io_fini(void)
{
    _child_iidx_io_fini();

    log_info("----------------------------------------------------------------");
    log_info("Printing histograms...");
    log_info("All values are times in microseconds (us)");

    log_info("----------------------------------------------------------------");
    log_info(">>> EP1 send");
    hdr_percentiles_print(_histogram_ep1_send, stdout, 5, 1.0, CLASSIC);
    log_info("----------------------------------------------------------------");
    log_info(">>> EP2 recv");
    hdr_percentiles_print(_histogram_ep2_recv, stdout, 5, 1.0, CLASSIC);
    log_info("----------------------------------------------------------------");
    log_info(">>> EP3 write 16seg");
    hdr_percentiles_print(_histogram_ep3_write_16seg, stdout, 5, 1.0, CLASSIC); 

    log_info("----------------------------------------------------------------");

    fflush(stdout);

    hdr_close(_histogram_ep1_send);
    hdr_close(_histogram_ep2_recv);
    hdr_close(_histogram_ep3_write_16seg);

    FreeLibrary(_child_iidx_io_module);
}

void iidx_io_ep1_set_deck_lights(uint16_t deck_lights)
{
    return _child_iidx_io_ep1_set_deck_lights(deck_lights);
}

void iidx_io_ep1_set_panel_lights(uint8_t panel_lights)
{
    return _child_iidx_io_ep1_set_panel_lights(panel_lights);
}

void iidx_io_ep1_set_top_lamps(uint8_t top_lamps)
{
    return _child_iidx_io_ep1_set_top_lamps(top_lamps);
}

void iidx_io_ep1_set_top_neons(bool top_neons)
{
    return _child_iidx_io_ep1_set_top_neons(top_neons);
}

bool iidx_io_ep1_send(void)
{
    hdr_timespec start_time;
    hdr_timespec end_time;
    int64_t diff_us;
    bool result;

    hdr_gettime(&start_time);

    result = _child_iidx_io_ep1_send();

    hdr_gettime(&end_time);
    diff_us = hdr_timespec_diff_us(&start_time, &end_time);

    hdr_record_value(_histogram_ep1_send, diff_us);

    return result;
}

bool iidx_io_ep2_recv(void)
{
    hdr_timespec start_time;
    hdr_timespec end_time;
    int64_t diff_us;
    bool result;

    hdr_gettime(&start_time);

    result = _child_iidx_io_ep2_recv();

    hdr_gettime(&end_time);
    diff_us = hdr_timespec_diff_us(&start_time, &end_time);

    hdr_record_value(_histogram_ep2_recv, diff_us);

    return result;
}

uint8_t iidx_io_ep2_get_turntable(uint8_t player_no)
{
    return _child_iidx_io_ep2_get_turntable(player_no);
}

uint8_t iidx_io_ep2_get_slider(uint8_t slider_no)
{
    return _child_iidx_io_ep2_get_slider(slider_no);
}

uint8_t iidx_io_ep2_get_sys(void)
{
    return _child_iidx_io_ep2_get_sys();
}

uint8_t iidx_io_ep2_get_panel(void)
{
    return _child_iidx_io_ep2_get_panel();
}

uint16_t iidx_io_ep2_get_keys(void)
{
    return _child_iidx_io_ep2_get_keys();
}

bool iidx_io_ep3_write_16seg(const char *text)
{
    hdr_timespec start_time;
    hdr_timespec end_time;
    int64_t diff_us;
    bool result;

    hdr_gettime(&start_time);

    result = _child_iidx_io_ep3_write_16seg(text);

    hdr_gettime(&end_time);
    diff_us = hdr_timespec_diff_us(&start_time, &end_time);

    hdr_record_value(_histogram_ep3_write_16seg, diff_us);

    return result;
}

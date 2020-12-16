#define LOG_MODULE "vigem-iidx-cab-light-sequencer"

#include <stdbool.h>
#include <stdint.h>

#include "util/log.h"
#include "util/time.h"

static const uint32_t _SEQ_CYCLE_TIME_MS = 2000;

static bool _enabled;

static uint64_t _time_counter;

void vigem_iidxio_cab_light_sequencer_init()
{
    _time_counter = time_get_counter();
    _enabled = true;

    if (_enabled) {
        log_info("Initialized");
    }
}

void vigem_iidxio_cab_light_sequencer_update(bool* out_neon, uint8_t* out_spots)
{
    log_assert(out_neon);
    log_assert(out_spots);

    if (!_enabled) {
        *out_neon = false;
        *out_spots = 0;
        return;
    }

    uint64_t counter_now = time_get_counter();
    uint32_t cycle_time_elapsed_ms = time_get_elapsed_ms(counter_now - _time_counter);

    if (cycle_time_elapsed_ms < _SEQ_CYCLE_TIME_MS / 2) {
        *out_neon = false;
    } else {
        *out_neon = true;
    }

    if (cycle_time_elapsed_ms >= _SEQ_CYCLE_TIME_MS) {
        _time_counter = counter_now;
    }
}
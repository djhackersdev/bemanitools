#define LOG_MODULE "vigem-iidx-cab-light-sequencer"

#include <stdbool.h>
#include <stdint.h>

#include "vigem-iidxio/cab-light-sequencer.h"

#include "util/log.h"
#include "util/time.h"

static const uint32_t _SEQ_CYCLE_TIME_MS = 2000;

static enum vigem_iidxio_cab_light_sequencer_mode _light_seq_mode;

static bool _first_update;
static uint64_t _time_counter;

static uint8_t _tt_prev[2];

void _update_neons_seq_flash(bool* out_neon)
{
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

void _update_neons_flash_tt_input(uint8_t tt_p1, uint8_t tt_p2, bool* out_neon)
{
    if (_first_update) {
        _tt_prev[0] = tt_p1;
        _tt_prev[1] = tt_p2;
        return;
    }

    if (_tt_prev[0] != tt_p1 || _tt_prev[1] != tt_p2) {
        *out_neon = true;
    } else {
        *out_neon = false;
    }
}

void vigem_iidxio_cab_light_sequencer_init(
        enum vigem_iidxio_cab_light_sequencer_mode light_seq_mode)
{
    _time_counter = time_get_counter();
    _light_seq_mode = light_seq_mode;

    _first_update = true;

    if (_light_seq_mode != LIGHT_SEQ_MODE_OFF) {
        log_info("Initialized");
    }
}

void vigem_iidxio_cab_light_sequencer_update(
        uint16_t keys,
        uint8_t tt_p1,
        uint8_t tt_p2,
        bool* out_neon,
        uint8_t* out_spots)
{
    log_assert(out_neon);
    log_assert(out_spots);

    switch (_light_seq_mode) {
        case LIGHT_SEQ_MODE_NEONS_FLASH:
            _update_neons_seq_flash(out_neon);

        case LIGHT_SEQ_MODE_NEONS_FLASH_TT_INPUT:
            _update_neons_flash_tt_input(tt_p1, tt_p2, out_neon);
            break;

        case LIGHT_SEQ_MODE_OFF:
            // fallthrough
        default:
            *out_neon = false;
            *out_spots = 0;
            break;
    }

    _first_update = false;
}

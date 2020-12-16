#define LOG_MODULE "vigem-iidx-cab-16seg-sequencer"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "util/log.h"
#include "util/time.h"

static const uint8_t _MAX_LEN_16SEG = 9;

static bool _enabled;

static uint64_t _time_counter;
static size_t _text_pos;

static char _text[1024 + 1];
static size_t _text_len;

static uint32_t _scroll_cycle_time_ms;

static void _create_display_string_with_wrap_around(char* out_16seg)
{
    size_t cur_text_pos = _text_pos;

    for (uint8_t i = 0; i < _MAX_LEN_16SEG; i++) {
        if (cur_text_pos >= _text_len) {
            cur_text_pos = 0;
        }
        
        out_16seg[i] = _text[cur_text_pos];
        cur_text_pos++;
    }
}

void vigem_iidxio_cab_16seg_sequencer_init(const char* text, uint32_t scroll_cycle_time_ms)
{
    log_assert(text);

    _text_len = strlen(text);

    if (_text_len + 1 > sizeof(_text)) {
        log_warning("Truncating input text as it exceeds the max size");
        strncpy(_text, text, sizeof(_text) - 1);
    } else {
        strcpy(_text, text);
    }

    _scroll_cycle_time_ms = scroll_cycle_time_ms;

    _time_counter = time_get_counter();
    _enabled = true;

    if (_enabled) {
        log_info("Initialized, cycle time %d ms, text \"%s\"", _scroll_cycle_time_ms, _text);
    }
}

void vigem_iidxio_cab_16seg_sequencer_update(char* out_16seg)
{
    log_assert(out_16seg);

    memset(out_16seg, ' ', _MAX_LEN_16SEG);

    if (!_enabled) {
        return;
    }

    uint64_t counter_now = time_get_counter();
    uint32_t cycle_time_elapsed_ms = time_get_elapsed_ms(counter_now - _time_counter);

    if (cycle_time_elapsed_ms >= _scroll_cycle_time_ms) {
        _time_counter = counter_now;
        _text_pos++;
    
        if (_text_pos >= _text_len) {
            _text_pos = 0;
        }
    }

    // Have static text if not exceeding 16seg display size
    if (_text_len <= _MAX_LEN_16SEG) {
        memcpy(out_16seg, _text, _text_len);
    } else {
        _create_display_string_with_wrap_around(out_16seg);
    }
}
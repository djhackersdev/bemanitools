#ifndef AC_IO_EMU_ICCA_H
#define AC_IO_EMU_ICCA_H

#include <stdbool.h>
#include <stdint.h>

#include "acioemu/emu.h"

struct ac_io_emu_icca {
    struct ac_io_emu *emu;
    uint8_t unit_no;
    bool fault;
    bool engaged;
    uint8_t last_sensor;
    uint16_t last_keypad;
    uint8_t key_events[2];
    uint8_t uid[8];
    uint8_t card_result;
    bool detected_new_reader;
    bool keypad_started;
    bool polling_started;
    uint64_t time_counter_last_poll;
};

void ac_io_emu_icca_init(
    struct ac_io_emu_icca *icca, struct ac_io_emu *emu, uint8_t unit_no);

void ac_io_emu_icca_dispatch_request(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req);

#endif

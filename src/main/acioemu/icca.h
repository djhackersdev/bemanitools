#ifndef AC_IO_EMU_ICCA_H
#define AC_IO_EMU_ICCA_H

#include <stdbool.h>
#include <stdint.h>

#include "acioemu/emu.h"

enum ac_io_emu_icca_version {
    v150 = 0x5,
    v160 = 0x6,
    v170 = 0x7,
};

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

    enum ac_io_emu_icca_version version;
    bool cipher_started;
    uint32_t cipher_keys[4];
};

void ac_io_emu_icca_init(
    struct ac_io_emu_icca *icca, struct ac_io_emu *emu, uint8_t unit_no);

// optional, call after init to override default version
void ac_io_emu_icca_set_version(
    struct ac_io_emu_icca *icca, enum ac_io_emu_icca_version version);

void ac_io_emu_icca_dispatch_request(
    struct ac_io_emu_icca *icca, const struct ac_io_message *req);

#endif

#ifndef SDVXHOOK2_BI2A_H
#define SDVXHOOK2_BI2A_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bio2emu/emu.h"

enum bio2_bi2a_cmd {
    // Custom Stuff
    BIO2_BI2A_CMD_UNK_0100 = 0x0100,
    BIO2_BI2A_CMD_UNK_0120 = 0x0120,
    BIO2_BI2A_CMD_POLL = 0x0152,
};

#pragma pack(push, 1)
struct bi2a_sdvx_analog {
    uint16_t unk1 : 1;
    uint16_t a_coin : 1;
    uint16_t a_service : 1;
    uint16_t a_test : 1;
    uint16_t unk2 : 2;
    uint16_t a_val : 10;
};

struct bi2a_sdvx_buttons1 {
    uint8_t b_fxl : 1;
    uint8_t b_d : 1;
    uint8_t b_c : 1;
    uint8_t b_b : 1;
    uint8_t b_a : 1;
    uint8_t b_start : 1;
    uint8_t b_recorder : 1;
    uint8_t b_headphone : 1;
};

struct bi2a_sdvx_buttons2 {
    uint8_t unk : 5;
    uint8_t b_ex2 : 1;
    uint8_t b_ex1 : 1;
    uint8_t b_fxr : 1;
};

struct bio2_bi2a_state {
    union {
        uint16_t raw[8];
        struct {
            struct bi2a_sdvx_analog analogs[4];
            uint8_t unk_1; // coin mech?
            struct bi2a_sdvx_buttons1 buttons_1;
            struct bi2a_sdvx_buttons2 buttons_2;
            uint8_t unk_2[5];
        };
    };
};

struct bio2_bi2a_state_out {
    uint8_t unk1[2];
    uint8_t c_block;
    uint8_t woof_r;
    uint8_t woof_g;
    uint8_t unk2;
    uint8_t woof_b;
    uint8_t controller[3];
    uint8_t unk3;
    uint8_t gpio[7]; // START, ABCD, FXL, FXR
    uint8_t unk4[3];
    uint8_t generator[3];
    uint8_t unk5[3];
    uint8_t gpio2[3]; // pop, titleL, titleR
    uint8_t unk6;
    uint8_t wingUpper[3];
    uint8_t wingLower[3];
    uint8_t unk7[3];
};

_Static_assert(
    sizeof(struct bio2_bi2a_state) == 16,
    "bio2_bi2a_state_in is the wrong size");
_Static_assert(
    sizeof(struct bio2_bi2a_state_out) == 40,
    "bio2_bi2a_state_out is the wrong size");
#pragma pack(pop)

void bio2_emu_bi2a_init(struct bio2emu_port *in, bool disable_poll_limiter);
void bio2_emu_bi2a_dispatch_request(
    struct bio2emu_port *bio2port, const struct ac_io_message *req);

#endif

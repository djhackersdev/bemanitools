#ifndef BIO2_BI2A_SDVX
#define BIO2_BI2A_SDVX

#include "bio2/bio2.h"

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

#endif

#ifndef BIO2_BI2A_DDR
#define BIO2_BI2A_DDR

#include "bio2/bio2.h"

#pragma pack(push, 1)

struct bi2a_ddr_system_panel {
    uint8_t b_unk1 : 1;
    uint8_t b_coin : 1;
    uint8_t b_service : 1;
    uint8_t b_test : 1;
    uint8_t b_un2 : 4;
};

struct bi2a_ddr_player_1_panel {
    uint8_t b_unk : 3;
    uint8_t b_right : 1;
    uint8_t b_left : 1;
    uint8_t b_down : 1;
    uint8_t b_up : 1;
    uint8_t b_start : 1;
};

struct bi2a_ddr_player_2_panel {
    uint8_t b_unk1 : 1;
    uint8_t b_right : 1;
    uint8_t b_left : 1;
    uint8_t b_down : 1;
    uint8_t b_up : 1;
    uint8_t b_start : 1;
    uint8_t b_unk2 : 2;
};

struct bi2a_ddr_state_in {
    uint8_t unk1;
    struct bi2a_ddr_system_panel system;
    uint8_t unk2[7];
    struct bi2a_ddr_player_1_panel player_1;
    struct bi2a_ddr_player_2_panel player_2;
    uint8_t unk3[5];
};

struct bi2a_ddr_state_out {
    uint8_t unk1[40];
};

_Static_assert(
    sizeof(struct bi2a_ddr_state_in) == 16,
    "bi2a_ddr_state_in is the wrong size");
_Static_assert(
    sizeof(struct bi2a_ddr_state_out) == 40,
    "bi2a_ddr_state_out is the wrong size");
#pragma pack(pop)

#endif

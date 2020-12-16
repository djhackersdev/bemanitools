#ifndef BIO2_BI2A_IIDX
#define BIO2_BI2A_IIDX

#include "bio2/bio2.h"

enum bi2a_iidx_panel_button {
    BI2A_IIDX_PANEL_BUTTON_START_P1 = 0,
    BI2A_IIDX_PANEL_BUTTON_START_P2 = 1,
    BI2A_IIDX_PANEL_BUTTON_VEFX = 2,
    BI2A_IIDX_PANEL_BUTTON_EFFECT = 3,
};

enum bi2a_iidx_spotlight_left {
    BI2A_IIDX_SPOTLIGHT_LEFT_BLUE = 0,
    BI2A_IIDX_SPOTLIGHT_LEFT_GREEN = 1,
    BI2A_IIDX_SPOTLIGHT_LEFT_YELLOW = 2,
    BI2A_IIDX_SPOTLIGHT_LEFT_RED = 3,
};

enum bi2a_iidx_spotlight_right {
    BI2A_IIDX_SPOTLIGHT_RIGHT_RED = 0,
    BI2A_IIDX_SPOTLIGHT_RIGHT_YELLOW = 1,
    BI2A_IIDX_SPOTLIGHT_RIGHT_GREEN = 2,
    BI2A_IIDX_SPOTLIGHT_RIGHT_BLUE = 3,
};

#pragma pack(push, 1)
struct bi2a_iidx_slider {
    uint8_t s_unk : 4;
    uint8_t s_val : 4;
};

struct bi2a_iidx_system {
    uint8_t v_unk1 : 1;
    uint8_t v_coin : 1;
    uint8_t v_service : 1;
    uint8_t v_test : 1;
    uint8_t v_unk2 : 4;
};

struct bi2a_iidx_button {
    uint8_t b_unk : 7;
    uint8_t b_val : 1;
};

struct bi2a_iidx_panel {
    uint8_t y_unk : 4;
    uint8_t y_effect : 1;
    uint8_t y_vefx : 1;
    uint8_t y_start2 : 1;
    uint8_t y_start1 : 1;
};

struct bi2a_iidx_state_in {
    struct bi2a_iidx_slider SLIDER1;
    struct bi2a_iidx_system SYSTEM;
    struct bi2a_iidx_slider SLIDER2;
    uint8_t UNK1;
    struct bi2a_iidx_slider SLIDER3;
    uint8_t UNK2;
    struct bi2a_iidx_slider SLIDER4;
    struct bi2a_iidx_slider SLIDER5;
    uint8_t coins; // coin mech?
    struct bi2a_iidx_panel PANEL;
    uint8_t UNK4;
    uint8_t UNK5;
    uint8_t UNK6;
    uint8_t UNK7;
    uint8_t UNK8;
    uint8_t UNK9;
    uint8_t TURNTABLE1;
    uint8_t TURNTABLE2;
    struct bi2a_iidx_button P1SW1;
    uint8_t UNK11;
    struct bi2a_iidx_button P1SW2;
    uint8_t UNK12;
    struct bi2a_iidx_button P1SW3;
    uint8_t UNK13;
    struct bi2a_iidx_button P1SW4;
    uint8_t UNK14;
    struct bi2a_iidx_button P1SW5;
    uint8_t UNK15;
    struct bi2a_iidx_button P1SW6;
    uint8_t UNK16;
    struct bi2a_iidx_button P1SW7;
    uint8_t UNK17;
    struct bi2a_iidx_button P2SW1;
    uint8_t UNK21;
    struct bi2a_iidx_button P2SW2;
    uint8_t UNK22;
    struct bi2a_iidx_button P2SW3;
    uint8_t UNK23;
    struct bi2a_iidx_button P2SW4;
    uint8_t UNK24;
    struct bi2a_iidx_button P2SW5;
    uint8_t UNK25;
    struct bi2a_iidx_button P2SW6;
    uint8_t UNK26;
    struct bi2a_iidx_button P2SW7;
    uint8_t UNK27;
};

struct bi2a_iidx_light {
    uint8_t l_unk : 7;
    uint8_t l_state : 1;
};

struct bi2a_iidx_state_out {
    struct bi2a_iidx_light UNK1[3];
    struct bi2a_iidx_light PANEL[4];
    struct bi2a_iidx_light DECKSW[14];
    uint8_t UNK2[2];
    uint8_t SEG16[9];
    // right
    struct bi2a_iidx_light SPOTLIGHT1[4];
    struct bi2a_iidx_light NEONLAMP;
    // left
    struct bi2a_iidx_light SPOTLIGHT2[4];
    uint8_t UNK3[7];
};
_Static_assert(
    sizeof(struct bi2a_iidx_state_out) == 48,
    "bi2a_iidx_state_out is the wrong size");
_Static_assert(
    sizeof(struct bi2a_iidx_state_in) == 46,
    "bi2a_iidx_state_in is the wrong size");
#pragma pack(pop)

#endif

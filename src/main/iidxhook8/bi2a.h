#ifndef IIDXHOOK_BI2A_H
#define IIDXHOOK_BI2A_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "bio2emu/emu.h"

enum bio2_iidx_cmd {
    // Custom Stuff
    BIO2_BI2A_CMD_UNK_0100         = 0x0100,
    BIO2_BI2A_CMD_UNK_0120         = 0x0120,
    BIO2_BI2A_CMD_POLL             = 0x0152,
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

struct bi2a_iidx_state_out {
    struct bi2a_iidx_slider SLIDER1;
    struct bi2a_iidx_system SYSTEM;
    struct bi2a_iidx_slider SLIDER2;
    uint8_t UNK1;
    struct bi2a_iidx_slider SLIDER3;
    uint8_t UNK2;
    struct bi2a_iidx_slider SLIDER4;
    struct bi2a_iidx_slider SLIDER5;
    uint8_t UNK3; // coin mech?
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

struct bi2a_iidx_state_in {
    struct bi2a_iidx_light UNK1[3];
    struct bi2a_iidx_light PANEL[4];
    struct bi2a_iidx_light DECKSW[14];
    uint8_t UNK2[2];
    uint8_t SEG16[9];
    struct bi2a_iidx_light SPOTLIGHT1[4];
    struct bi2a_iidx_light NEONLAMP;
    struct bi2a_iidx_light SPOTLIGHT2[4];
    uint8_t UNK3[7];
};
_Static_assert(sizeof(struct bi2a_iidx_state_in) == 48, "bi2a_iidx_state_in is the wrong size");
_Static_assert(sizeof(struct bi2a_iidx_state_out) == 46, "bi2a_iidx_state_out is the wrong size");
#pragma pack(pop)

void bio2_emu_bi2a_init(struct bio2emu_port *in, bool disable_poll_limiter);
void bio2_emu_bi2a_dispatch_request(struct bio2emu_port *bio2port, const struct ac_io_message *req);

#endif

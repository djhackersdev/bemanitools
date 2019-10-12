#ifndef IIDXHOOK_BI2A_H
#define IIDXHOOK_BI2A_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "acioemu/emu.h"

enum bio2_bi2a_cmd {
    // Custom Stuff
    BIO2_BI2A_CMD_UNK_0100         = 0x0100,
    BIO2_BI2A_CMD_UNK_0120         = 0x0120,
    BIO2_BI2A_CMD_POLL             = 0x0152,
};

#pragma pack(push, 1)
struct _slider {
   uint8_t s_unk : 4;
   uint8_t s_val : 4;
};

struct _system {
   uint8_t v_unk1 : 1;
   uint8_t v_coin : 1;
   uint8_t v_service : 1;
   uint8_t v_test : 1;
   uint8_t v_unk2 : 4;
};

struct _button {
   uint8_t b_unk : 7;
   uint8_t b_val : 1;
};

struct _panel {
   uint8_t y_unk : 4;
   uint8_t y_effect : 1;
   uint8_t y_vefx : 1;
   uint8_t y_start2 : 1;
   uint8_t y_start1 : 1;
};

struct bio2_bi2a_state {
    struct _slider SLIDER1;
    struct _system SYSTEM;
    struct _slider SLIDER2;
    uint8_t UNK1;
    struct _slider SLIDER3;
    uint8_t UNK2;
    struct _slider SLIDER4;
    struct _slider SLIDER5;
    uint8_t UNK3; // coin mech?
    struct _panel PANEL;
    uint8_t UNK4;
    uint8_t UNK5;
    uint8_t UNK6;
    uint8_t UNK7;
    uint8_t UNK8;
    uint8_t UNK9;
    uint8_t TURNTABLE1;
    uint8_t TURNTABLE2;
    struct _button P1SW1;
    uint8_t UNK11;
    struct _button P1SW2;
    uint8_t UNK12;
    struct _button P1SW3;
    uint8_t UNK13;
    struct _button P1SW4;
    uint8_t UNK14;
    struct _button P1SW5;
    uint8_t UNK15;
    struct _button P1SW6;
    uint8_t UNK16;
    struct _button P1SW7;
    uint8_t UNK17;
    struct _button P2SW1;
    uint8_t UNK21;
    struct _button P2SW2;
    uint8_t UNK22;
    struct _button P2SW3;
    uint8_t UNK23;
    struct _button P2SW4;
    uint8_t UNK24;
    struct _button P2SW5;
    uint8_t UNK25;
    struct _button P2SW6;
    uint8_t UNK26;
    struct _button P2SW7;
    uint8_t UNK27;
};

struct _light {
   uint8_t l_unk : 7;
   uint8_t l_state : 1;
};

struct bio2_bi2a_state_in {
    struct _light UNK1[3];
    struct _light PANEL[4];
    struct _light DECKSW[14];
    uint8_t UNK2[2];
    uint8_t SEG16[9];
    struct _light SPOTLIGHT1[4];
    struct _light NEONLAMP;
    struct _light SPOTLIGHT2[4];
    uint8_t UNK3[7];
};
_Static_assert(sizeof(struct bio2_bi2a_state_in) == 48, "bio2_bi2a_state_in is the wrong size");
_Static_assert(sizeof(struct bio2_bi2a_state) == 46, "bio2_bi2a_state_out is the wrong size");
#pragma pack(pop)

struct bio2emu_port;

void bio2_emu_bi2a_init(struct bio2emu_port *in, bool disable_poll_limiter);
void bio2_emu_bi2a_dispatch_request(struct bio2emu_port *bio2port, const struct ac_io_message *req);

#endif

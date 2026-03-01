#ifndef P4IO_BITINFO_H
#define P4IO_BITINFO_H

#pragma pack(push, 1)

typedef union {
    struct {
        bool coin0_0 : 1;
        bool coin0_1 : 1;
        bool p2_leftright : 1;
        bool p2_updown : 1;
        bool p2_start : 1;
        bool coin0_5 : 1;
        bool coin0_6 : 1;
        bool coin_blocker : 1;

        bool coin1_0 : 1;
        bool coin1_1 : 1;
        bool coin1_2 : 1;
        bool coin1_3 : 1;
        bool coin1_4 : 1;
        bool coin1_5 : 1;
        bool coin1_6 : 1;
        bool coin1_7 : 1;

        bool coin2_0 : 1;
        bool coin2_1 : 1;
        bool coin2_2 : 1;
        bool coin2_3 : 1;
        bool coin2_4 : 1;
        bool coin2_5 : 1;
        bool coin2_6 : 1;
        bool coin2_7 : 1;

        bool coin3_0 : 1;
        bool coin3_1 : 1;
        bool coin3_2 : 1;
        bool coin3_3 : 1;
        bool coin3_4 : 1;
        bool coin3_5 : 1;
        bool coin3_6 : 1;
        bool coin3_7 : 1;
    } ddr;
    uint8_t raw[4];
} p4io_coinstock_t;
_Static_assert(
    sizeof(p4io_coinstock_t) == 4, "p4io_coinstock_t is the wrong size");

typedef union {
    struct {
        uint8_t header_up_g;
        uint8_t header_up_r;
        uint8_t header_up_b;

        uint8_t header_down_g;
        uint8_t header_down_r;
        uint8_t header_down_b;

        uint8_t bass_g;
        uint8_t bass_r;
        uint8_t bass_b;

        uint8_t p1_start;
        uint8_t p1_updown;
        uint8_t p1_leftright;

        uint8_t light12;
        uint8_t light13;
        uint8_t light14;
        uint8_t light15;
    } ddr;
    uint8_t raw[16];
} p4io_lights_t;

_Static_assert(sizeof(p4io_lights_t) == 16, "p4io_lights_t is the wrong size");

#pragma pack(pop)

#endif
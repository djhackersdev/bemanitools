#ifndef AC_IO_RVOL_H
#define AC_IO_RVOL_H

#include <stdint.h>

// technically RVOL also can poll with CMD 0x0113
// but museca never uses this (it's controlled by if the watchdog is on/off)
#define AC_IO_CMD_RVOL_POLL 0x0112
#define AC_IO_CMD_RVOL_SET_EXPAND_MODE 0x0114
#define AC_IO_CMD_RVOL_AMP_CONTROL 0x0128

#pragma pack(push, 1)

struct ac_io_rvol_buttons_1 {
    uint8_t unk0 : 1;
    uint8_t unk1 : 1;
    uint8_t spinner_4 : 1;
    uint8_t pedal : 1;
    uint8_t spinner_5 : 1;
    uint8_t unk5 : 1;
    uint8_t unk6 : 1;
    uint8_t unk7 : 1;
};

struct ac_io_rvol_buttons_2 {
    uint8_t spinner_2 : 1;
    uint8_t unk1 : 1;
    uint8_t unk2 : 1;
    uint8_t spinner_1 : 1;
    uint8_t unk4 : 1;
    uint8_t unk5 : 1;
    uint8_t unk6 : 1;
    uint8_t unk7 : 1;
};

struct ac_io_rvol_buttons_3 {
    uint8_t unk0 : 1;
    uint8_t unk1 : 1;
    uint8_t unk2 : 1;
    uint8_t spinner_3 : 1;
    uint8_t unk4 : 1;
    uint8_t unk5 : 1;
    uint8_t unk6 : 1;
    uint8_t unk7 : 1;
};

struct ac_io_rvol_poll_in {
    uint8_t unk_0;
    struct ac_io_rvol_buttons_1 buttons_1;
    uint8_t unk_2_8[7];
    struct ac_io_rvol_buttons_2 buttons_2;
    uint8_t unk_10;
    struct ac_io_rvol_buttons_3 buttons_3;
    uint8_t unk_12_15[4];
    uint8_t spinners[5];
    uint8_t unk_21_22[2];
};

struct light_rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct ac_io_rvol_poll_out {
    uint8_t unk0_3[4];
    struct light_rgb spinner[5];
    struct light_rgb title;
    uint8_t unk22_31[10];
};

#pragma pack(pop)

#endif

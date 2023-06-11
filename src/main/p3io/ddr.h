#ifndef P3IO_DDR_H
#define P3IO_DDR_H

#include <stdint.h>

#pragma pack(push, 1)

struct p3io_ddr_player {
    uint8_t menu_start : 1;
    uint8_t pad_up : 1;
    uint8_t pad_down : 1;
    uint8_t pad_left : 1;
    uint8_t pad_right : 1;
    uint8_t unknown : 1;
    uint8_t menu_left : 1;
    uint8_t menu_right : 1;
};

struct p3io_ddr_operator {
    // Yes, this also contains the HD cab menu up and down buttons
    // which are not "operator" inputs. I suppose they just had to go
    // somewhere ¯\_(ツ)_/¯
    uint8_t p1_menu_up : 1;
    uint8_t p1_menu_down : 1;
    uint8_t p2_menu_up : 1;
    uint8_t p2_menu_down : 1;
    uint8_t test : 1;
    uint8_t coin : 1;
    uint8_t service : 1;
    uint8_t unknown : 1;
};

struct p3io_ddr_jamma {
    uint8_t unknown_80;

    struct p3io_ddr_player p1;
    struct p3io_ddr_player p2;
    struct p3io_ddr_operator operator;

    uint32_t unused;
    uint32_t unused2;
};

struct p3io_ddr_cabinet_light {
    uint8_t p1_menu_buttons : 1;
    uint8_t p2_menu_buttons : 1;
    uint8_t unknown : 2;
    uint8_t top_p1_lower : 1;
    uint8_t top_p1_upper : 1;
    uint8_t top_p2_lower : 1;
    uint8_t top_p2_upper : 1;
};

struct p3io_ddr_output {
    uint8_t unused_00[3];
    struct p3io_ddr_cabinet_light cabinet;
};

_Static_assert(
    sizeof(struct p3io_ddr_player) == 1,
    "struct p3io_ddr_player is the wrong size");

_Static_assert(
    sizeof(struct p3io_ddr_operator) == 1,
    "struct p3io_ddr_operator is the wrong size");

_Static_assert(
    sizeof(struct p3io_ddr_jamma) == sizeof(uint32_t) * 3,
    "struct p3io_ddr_jamma is the wrong size");

_Static_assert(
    sizeof(struct p3io_ddr_cabinet_light) == 1,
    "struct p3io_ddr_cabinet_light is the wrong size");

_Static_assert(
    sizeof(struct p3io_ddr_output) == sizeof(uint32_t),
    "struct p3io_ddr_output is the wrong size");

#pragma pack(pop)

#endif
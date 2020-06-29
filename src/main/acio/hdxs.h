#ifndef ACIO_HDXS_H
#define ACIO_HDXS_H

#include <stdint.h>

enum ac_io_hdxs_cmd {
    AC_IO_HDXS_CMD_SET_OUTPUTS = 0x0112,
};

#pragma pack(push, 1)
struct ac_io_hdxs_light {
    uint8_t analog : 7; // rgb component
    uint8_t bit : 1; // buttons
};

struct ac_io_hdxs_output {
    uint8_t empty;
    struct ac_io_hdxs_light lights[12];
};

enum ac_io_hdxs_front_panel_lights {
    AC_IO_HDXS_OUT_P1_START = 0,
    AC_IO_HDXS_OUT_P1_UP_DOWN = 1,
    AC_IO_HDXS_OUT_P1_LEFT_RIGHT = 2,
    AC_IO_HDXS_OUT_P2_START = 3,
    AC_IO_HDXS_OUT_P2_UP_DOWN = 4,
    AC_IO_HDXS_OUT_P2_LEFT_RIGHT = 5,
};

enum ac_io_hdxs_grb_layout {
    AC_IO_HDXS_GREEN = 0,
    AC_IO_HDXS_RED = 1,
    AC_IO_HDXS_BLUE = 2,
};

enum ac_io_hdxs_speaker_lights {
    AC_IO_HDXS_OUT_P1_SPEAKER_F = 0,
    AC_IO_HDXS_OUT_P2_SPEAKER_F = 1,
    AC_IO_HDXS_OUT_P1_SPEAKER_W = 2,
    AC_IO_HDXS_OUT_P2_SPEAKER_W = 3,
};

_Static_assert(
    sizeof(struct ac_io_hdxs_output) == 13,
    "ac_io_hdxs_output is the wrong size");
#pragma pack(pop)

#endif

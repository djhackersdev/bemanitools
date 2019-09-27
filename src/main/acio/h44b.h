#ifndef ACIO_H44B_H
#define ACIO_H44B_H

#include <stdint.h>

enum ac_io_h44b_cmd {
    AC_IO_H44B_CMD_SET_OUTPUTS = 0x0122,
};

struct ac_io_h44b_output {
    uint8_t front_rgb[3];
    uint8_t top_rgb[3];
    uint8_t left_rgb[3];
    uint8_t right_rgb[3];
    uint8_t title_rgb[3];
    uint8_t woofer_rgb[3];
};

#endif